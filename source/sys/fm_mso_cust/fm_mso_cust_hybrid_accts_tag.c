/******************************************************************************************
 *
 * Copyright (c) 2019 Hathway. All rights reserved.
 *
 * This material is the confidential property of Hathway 
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid DexYP license or sublicense agreement.
 *
 ****************************************************************************************/
/*****************************************************************************************
 * Maintentance Log:
 *
 * Date: 20190306
 * Author: Tata Consultancy Services 
 * Identifier: Hybrid Accounts (ISP & CATV / CATV & ISP) Accounts Tagging 
 * Notes: Hybrid Account Tagging / Detagging / Retagging 
 ****************************************************************************************/
/****************************************************************************************
 * File History
 *
 * DATE     |  CHANGE                                           |  Author
 ***********|***************************************************|************************
 |03/06/2019|Initial vesion					| Rama Puppala
 |26/12/2019|Added functionality for Hybrid account Tagging,	| Jyothirmayi Kachiraju
	    |Detagging and re-tagging of CATV / ISP accounts 	| 
 |**********|***************************************************|************************
 ****************************************************************************************
 * Opcode Name: MSO_OP_CUST_HYBRID_ACCTS_TAG
 * Opcode # : 13208
 * Functionality: Enables Hybrid Account Parent-Child Tagging, De-tagging and Re-tagging 
 *		  along with creation of Life Cycle Events for each action
 * Input Specification:

	--- Tagging the Hybrid Accounts
	0 PIN_FLD_POID 			POID [0] 0.0.0.1 /account -1 	// Mandatory. Dummy Account Poid
	0 PIN_FLD_USERID 		POID [0] NULL 			// Optional. User Account Poid
	0 PIN_FLD_ACCOUNT_NO 	 	 STR [0] "1121825794"  		// Mandatory. Main Account Number
	0 PIN_FLD_ACCOUNT_TAG 	 	 STR [0] "1147525320"  		// Mandatory. Account Number to be tagged to Main Account Number
	0 PIN_FLD_FLAGS		 	 INT [0] 1 			// Mandatory. Flag for tagging Hybrid Accounts
	0 PIN_FLD_PROGRAM_NAME 	 	 STR [0] "OAP|sushil.sp"	// Mandatory. Program name
	0 MSO_FLD_STB_ID		 STR [0] ""  			// Optional. But if this is there, PIN_FLD_OFFER_NAME for tagging is mandatory 
	0 MSO_FLD_VC_ID 		 STR [0] ""  			// Optional. But if this is there, PIN_FLD_OFFER_NAME for tagging is mandatory
	0 PIN_FLD_OFFER_NAME	 	 STR [0] ""  			// Optional. If MSO_FLD_STB_ID or MSO_FLD_VC_ID is there, PIN_FLD_OFFER_NAME is mandatory

	--- De-Tagging the Hybrid Accounts
	0 PIN_FLD_POID 			POID [0] 0.0.0.1 /account -1 	// Mandatory. Dummy Account Poid
	0 PIN_FLD_USERID 		POID [0] NULL  			// Optional. User Account Poid
	0 PIN_FLD_ACCOUNT_NO 	 	 STR [0] "1107572565"  		// Mandatory. Main Account Number
	0 PIN_FLD_ACCOUNT_TAG 	 	 STR [0] "4524165878"  		// Mandatory. Account Number to be Detagged from the Main Account Number
	0 PIN_FLD_FLAGS		 	 INT [0] 2 			// Mandatory. Flag for de-tagging Hybrid Accounts
	0 PIN_FLD_PROGRAM_NAME 	 	 STR [0] "OAP|sushil.sp" 	// Mandatory. Program name

	--- Re-Tagging the Hybrid Accounts
	0 PIN_FLD_POID 			POID [0] 0.0.0.1 /account -1 	// Mandatory. Dummy Account Poid
	0 PIN_FLD_USERID 		POID [0] NULL 			// Optional. User Account Poid
	0 PIN_FLD_ACCOUNT_NO 	 	 STR [0] "1107572565"  		// Account Number which needs to be detagged from the Main Account Number
	0 PIN_FLD_ACCOUNT_TAG 	 	 STR [0] "4524165878"  		// Account Number to be tagged to Main Account Number as part of retagging
	0 PIN_FLD_FLAGS		 	 INT [0] 3 			// Mandatory. Flag for re-tagging Hybrid Accounts
	0 PIN_FLD_PROGRAM_NAME 	 	 STR [0] "OAP|sushil.sp" 	// Mandatory. Program name
	0 MSO_FLD_STB_ID		 STR [0] ""  			// Optional. But if this is there, PIN_FLD_OFFER_NAME for the retagging is mandatory
	0 MSO_FLD_VC_ID 		 STR [0] ""  			// Optional. But if this is there, PIN_FLD_OFFER_NAME for the retagging is mandatory
	0 PIN_FLD_OFFER_NAME	 STR [0] "" 				// Optional. If MSO_FLD_STB_ID or MSO_FLD_VC_ID is there, PIN_FLD_OFFER_NAME is mandatory

* Output Specification: (If Status is 0, the opcode call is successful, other than 0 is failure)

	Success Case:

	0 PIN_FLD_POID 			POID [0] 0.0.0.1 /account 9348818461
	0 PIN_FLD_DESCR			 STR [0] "Hybrid accounts re-tagging is successful !"
	0 PIN_FLD_STATUS		ENUM [0] 0	

	Failure Case: 

	0 PIN_FLD_POID 			POID [0] 0.0.0.1 /account 9348818461
	0 PIN_FLD_ERROR_CODE		 STR [0] "92022"
	0 PIN_FLD_ERROR_DESCR		 STR [0] "Tagging Account Write Flds Failed !"
	0 PIN_FLD_STATUS        	ENUM [0] 13
	
 ****************************************************************************************/

#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_cust_hybrid_accts_tag.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <pcm.h>
#include <pinlog.h>
#include "ops/device.h"
#include "pin_device.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "mso_device.h"
#include "mso_prov.h"
#include "mso_cust.h"
#include "mso_ops_flds.h"
#include "mso_lifecycle_id.h"
#include "mso_errs.h"

#define FILE_LOGNAME "fm_mso_cust_hybrid_accts_tag.c"

/*******************************************************************************
 * Functions Defined outside this source file
 ******************************************************************************/
/******************************************************************************
 * This function gets the account information based on the account number
 ******************************************************************************/
PIN_IMPORT void
fm_mso_get_acnt_from_acntno(
	pcm_context_t	*ctxp,
	char		*acnt_no,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT int32
fm_mso_trans_open(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        int32           flag,
        pin_errbuf_t    *ebufp);

PIN_IMPORT void
fm_mso_trans_commit(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT void
fm_mso_trans_abort(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

/******************************************************************************
 * This function gets the LCO City based on Account Poid
 ******************************************************************************/
PIN_EXPORT void
fm_mso_get_lco_city(
	pcm_context_t	*ctxp,
	poid_t		*account_pdp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

/******************************************************************************
 * This function creates the Life Cycle event based on an action
 ******************************************************************************/	
extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	*out_flistp,
	int32		action,
	pin_errbuf_t	*ebufp);

/*******************************************************************************
 * This function gets the billinfo_id based on the Account Object
 ******************************************************************************/
extern
void fm_mso_get_acct_billinfo(
	pcm_context_t	*ctxp,
	poid_t		*acnt_pdp,
	pin_flist_t	**bill_infop,
	pin_errbuf_t	*ebufp);
	
/************************************************************************
 * This function is validates the given BusinessType and BillInfoID of 
 * and returns the AccountType. 
 ************************************************************************/
extern 
void validate_hybrid_acnt_type(
	pcm_context_t	*ctxp,
	int32 		*business_typep,
	char		*billinfo_idp,
	char		**acnt_type,
	pin_errbuf_t	*ebufp);

/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_cust_hybrid_accts_tag(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

/************************************************************************
 * This function is used for tagging the ISP / CATV accounts with  
 * the select CATV / ISP accounts with each other.
 ************************************************************************/
static void
fm_mso_cust_hybrid_accts_tag(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	int32		flags,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);
	
/************************************************************************
 * This function is used for de-tagging an already tagged ISP / CATV   
 * account from the Hybrid Account
 ************************************************************************/
static void
fm_mso_cust_hybrid_accts_detag(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	int32		flags,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

/************************************************************************
 * This function is used for de-tagging an already tagged ISP / CATV   
 * account from the existing Hybrid Account and re-tagging the same 
 * account to another Hybrid account.
 ************************************************************************/
static void
fm_mso_cust_hybrid_accts_retag(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	int32		flags,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

/******************************************************************************************
 * Opcode Implementation Starts here 
 *****************************************************************************************/
void
op_mso_cust_hybrid_accts_tag(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	int32			*status = NULL;
	int32			*action_flag = NULL ;
	poid_t         		*pdp = NULL;	
	char			*main_acc_nop = NULL;
	char			*tag_acc_nop = NULL;
	pin_flist_t		*ret_flistp = NULL;
	int32			tagging_failure = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
       		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*************************************************
	 * Insanity check.
	 ************************************************/
	if (opcode != MSO_OP_CUST_HYBRID_ACCTS_TAG) 
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_mso_cust_hybrid_accts_tag", ebufp);
        	return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"op_mso_cust_hybrid_accts_tag input flist", i_flistp);
		
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	main_acc_nop = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	tag_acc_nop = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_TAG, 1, ebufp);
	action_flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	
	if ( !pdp || !main_acc_nop || !tag_acc_nop || !action_flag )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &tagging_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, HYBRID_ACCTS_MANDFLDS_MISS, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, HYBRID_ACCTS_MANDFLDS_MISS_DESC, ebufp);
		*o_flistpp = ret_flistp;
		return;		
	}
		
	if(action_flag && *(int32 *)action_flag == MSO_HYBRID_TAGGING)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Hybrid Account Tagging.");
		fm_mso_cust_hybrid_accts_tag(ctxp, i_flistp, flags, o_flistpp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"op_mso_cust_hybrid_accts_tag Hybrid Account Tagging: Error", ebufp);
				
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &tagging_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_DESC, ebufp);
			*o_flistpp = ret_flistp;
			return;	
		}
	}
	else if(action_flag && *(int32 *)action_flag == MSO_HYBRID_DETAGGING)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Hybrid Account De-Tagging.");
		fm_mso_cust_hybrid_accts_detag(ctxp, i_flistp, flags, o_flistpp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"op_mso_cust_hybrid_accts_tag Hybrid Account De-Tagging: Error", ebufp);
								
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &tagging_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, HYBRID_DTAG_ERR, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, HYBRID_DTAG_ERR_DESC, ebufp);
			*o_flistpp = ret_flistp;
			return;	
		}
	}
	else if(action_flag && *(int32 *)action_flag == MSO_HYBRID_RETAGGING)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Hybrid Account Re-Tagging");
		fm_mso_cust_hybrid_accts_retag(ctxp, i_flistp, flags, o_flistpp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"op_mso_cust_hybrid_accts_tag Hybrid Account Re-Tagging: Error", ebufp);
			
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &tagging_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_DESC, ebufp);
			*o_flistpp = ret_flistp;
			return;	
		}
	}

	if (*o_flistpp)
	{
		status = PIN_FLIST_FLD_GET(*o_flistpp, PIN_FLD_STATUS, 1, ebufp);
	}

	if (PIN_ERRBUF_IS_ERR(ebufp) && (status && (*(int*)status != 0))) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"op_mso_cust_hybrid_accts_tag error", ebufp);
    	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"op_mso_cust_hybrid_accts_tag return flist", *o_flistpp);

    return;
}

/*******************************************************************************
 * fm_mso_cust_hybrid_accts_tag()
 *
 * Inputs: flist with POID, ACCOUNT_NO and ACCOUNT_TAG are mandatory fields 

 * Output: void; ebuf set if errors encountered
 *
 * Description:
 * This function retrieves account poid of the given account number
 * and tags with the tagging account number and scheme code. 
 ******************************************************************************/
static void
fm_mso_cust_hybrid_accts_tag(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	int32		flags,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*acnt_infop = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*write_iflistp = NULL;
	pin_flist_t	*write_oflistp = NULL;
	pin_flist_t	*device_r_flistp = NULL;
	pin_flist_t	*serv_flistp = NULL;
	pin_flist_t	*lcevt_ret_flistp = NULL;
	pin_flist_t	*bill_infop = NULL;
	pin_flist_t	*r_flistp = NULL;
	
	poid_t         	*main_acc_pdp = NULL;
	poid_t         	*tag_acc_pdp = NULL;
	poid_t         	*serv_pdp = NULL;

	char		*main_acc_nop = NULL;
	char		*tag_acc_nop = NULL;
	char		*devicep = NULL;
	char		*offer_namep = NULL;
	char		*mainact_billinfo_idp = NULL;
	char		*tagact_billinfo_idp = NULL;
	char		*mainact_account_tagp = NULL;
	char		*tagact_account_tagp = NULL;
	char		*main_acnt_typep = NULL;
	char		*tag_acnt_typep = NULL;

	int32		*mainact_business_typep = NULL;
	int32		*tagact_business_typep = NULL;
	int32		ret_status = 0;	
	int32		t_status = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/*************************************************************
	 * Get the inputs from the Input Flist
	 *************************************************************/
	main_acc_nop = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	tag_acc_nop = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_TAG, 1, ebufp);

	/*************************************************************
	 * Validate the inputs in the Input Flist
	 ************************************************************/
	if ( (main_acc_nop && strlen(main_acc_nop) == 10) && (tag_acc_nop && strlen(tag_acc_nop) == 10) )
	{
		/****************************************************************
		 * Compare if the Main Account No and Tagging Account are same.
		 ****************************************************************/
		if(strcmp(main_acc_nop, tag_acc_nop) == 0)
		{
			ret_status = 2;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_1_DESC);
			goto CLEANUP;
		}
		
		/****************************************************************
		 * Get the Account details of the Main account
		 ****************************************************************/
		fm_mso_get_acnt_from_acntno(ctxp, main_acc_nop, &acnt_infop, ebufp);
		if (acnt_infop)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Main Account Details ");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Main Account Details flist", acnt_infop);
			main_acc_pdp = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_POID, 0, ebufp);	
			mainact_business_typep = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
			mainact_account_tagp = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_AAC_PACKAGE, 0, ebufp);
			PIN_FLIST_DESTROY_EX(&acnt_infop, ebufp);
		}
		else
		{
			ret_status = 3;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_2_DESC);
			goto CLEANUP;
		}
	
	
		if(!PIN_POID_IS_NULL(main_acc_pdp))
		{
			t_status = fm_mso_trans_open(ctxp, main_acc_pdp, 1, ebufp);

			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, main_acc_pdp, ebufp);
				
			/****************************************************************
			 * Check the Business Type of Main Account 
			 ****************************************************************/
			if (mainact_business_typep != NULL && *mainact_business_typep == 0)
			{
				ret_status = 4;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_3_DESC);
				goto CLEANUP;
			}
			
			/****************************************************************
			 * Get the BillInfoID Type of Main Account 
			 ****************************************************************/
			fm_mso_get_acct_billinfo(ctxp, main_acc_pdp, &bill_infop, ebufp);
			if (bill_infop)
			{
				mainact_billinfo_idp = PIN_FLIST_FLD_TAKE(bill_infop, PIN_FLD_BILLINFO_ID, 0, ebufp);
				PIN_FLIST_DESTROY_EX(&bill_infop, ebufp);
			}
			else
			{
				ret_status = 5;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_4_DESC);
				goto CLEANUP;
			}
		}

		/****************************************************************
		 * Get the Account details of the Tagging account
		 ****************************************************************/
		fm_mso_get_acnt_from_acntno(ctxp, tag_acc_nop, &acnt_infop, ebufp);
		if (acnt_infop)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Tagging Account Details ");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Tagging Account Details flist", acnt_infop);
			tag_acc_pdp = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_POID, 0, ebufp); 
			tagact_business_typep = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_BUSINESS_TYPE, 0, ebufp);				
			tagact_account_tagp = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_AAC_PACKAGE, 0, ebufp);
			PIN_FLIST_DESTROY_EX(&acnt_infop, ebufp);
		}
		else
		{
			ret_status = 6;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_5_DESC);
			goto CLEANUP;
		}
		
		if(!PIN_POID_IS_NULL(tag_acc_pdp))
		{
			/****************************************************************
			 * Get the Business Type of Tagging Account 
			 ****************************************************************/
			if (tagact_business_typep != NULL && *tagact_business_typep == 0)
			{
				ret_status = 7;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_6_DESC);
				goto CLEANUP;
			}
			
			/****************************************************************
			 * Get the BillInfoID Type of Tagging Account 
			 ****************************************************************/
			fm_mso_get_acct_billinfo(ctxp, tag_acc_pdp, &bill_infop, ebufp);
					
			if (bill_infop)
			{
				tagact_billinfo_idp = PIN_FLIST_FLD_TAKE(bill_infop, PIN_FLD_BILLINFO_ID, 0, ebufp);
				PIN_FLIST_DESTROY_EX(&bill_infop, ebufp);
			}
			else
			{
				ret_status = 8;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_7_DESC);
				goto CLEANUP;
			}
		}
		
		/**********************************************************************
		 * Validate if the Main Account already has any account tagged to it.
		 **********************************************************************/	
		if (mainact_account_tagp && strcmp(mainact_account_tagp, tag_acc_nop) == 0)
		{
			ret_status = 21;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_20_DESC);
			goto CLEANUP;
		}
		
		if (mainact_account_tagp && strcmp(mainact_account_tagp, "") != 0 && strcmp(mainact_account_tagp, tag_acc_nop) != 0)
		{
			ret_status = 9;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_8_DESC);
			goto CLEANUP;
		}
		
		/***************************************************************************
		 * Validate if the Tagging Account already has any account tagged to it.
		 ***************************************************************************/
		if (tagact_account_tagp && strcmp(tagact_account_tagp, main_acc_nop) == 0)
		{
			ret_status = 22;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_21_DESC);
			goto CLEANUP;
		}
			
		if (tagact_account_tagp && strcmp(tagact_account_tagp, "") != 0 && strcmp(tagact_account_tagp, main_acc_nop) != 0)
		{
			ret_status = 10;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_9_DESC);
			goto CLEANUP;
		}
		
		/***********************************************************************************
		 * If Main Account and Tagging Account have the same Bill_Info_ID, then throw error
		 ***********************************************************************************/	
		if ( mainact_billinfo_idp && tagact_billinfo_idp &&
			 strcmp(mainact_billinfo_idp, tagact_billinfo_idp) == 0 )
		{
			ret_status = 11;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_10_DESC);
			goto CLEANUP;
		}
		
		/***********************************************************************************
		 * If Main Account and Tagging Account have the same BusinessType, then throw error
		 ***********************************************************************************/	
		if (mainact_business_typep != NULL && tagact_business_typep != NULL &&
			 *mainact_business_typep == *tagact_business_typep)
		{
			ret_status = 12;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_11_DESC);
			goto CLEANUP;
		}
						
		/***********************************************
		 * Validate the AccountType of Main Account
		 ***********************************************/	
		 validate_hybrid_acnt_type(ctxp, mainact_business_typep, mainact_billinfo_idp, &main_acnt_typep, ebufp);		 
		 if ( PIN_ERRBUF_IS_ERR(ebufp) || (main_acnt_typep && strcmp(main_acnt_typep, ACTTYPE_INVALID) == 0) ) 
		 {
			ret_status = 13;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_12_DESC);
			goto CLEANUP;
		 }
		 
		 /***********************************************
		 * Validate the AccountType of Tagging Account
		 ***********************************************/
		 validate_hybrid_acnt_type(ctxp, tagact_business_typep, tagact_billinfo_idp, &tag_acnt_typep, ebufp);
		 if ( PIN_ERRBUF_IS_ERR(ebufp) || (tag_acnt_typep && strcmp(tag_acnt_typep, ACTTYPE_INVALID) == 0) ) 
		 {
			ret_status = 14;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_13_DESC);
			goto CLEANUP;
		 }
		 
		 /******************************************************************************************
		 * If Main Account and Tagging Account have same Account Types, then throw error
		 ******************************************************************************************/			
		if (main_acnt_typep && tag_acnt_typep && strcmp(main_acnt_typep, tag_acnt_typep) == 0)
		{
			ret_status = 15;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_14_DESC);
			goto CLEANUP;
		}
		
		if ( main_acnt_typep && tag_acnt_typep && 
				 (strcmp(main_acnt_typep, ACTTYPE_ISP) == 0 && strcmp(tag_acnt_typep, ACTTYPE_CATV) == 0) ||
				 (strcmp(main_acnt_typep, ACTTYPE_CATV) == 0 && strcmp(tag_acnt_typep, ACTTYPE_ISP) == 0) )
		{		
			if (!PIN_POID_IS_NULL(main_acc_pdp) && !PIN_POID_IS_NULL(tag_acc_pdp))
			{
				/**************************************************************************
				 * Update AAC_PACKAGE of the Main Account with the Tagging Account number
				 *************************************************************************/
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating AAC_PACKAGE of the Main Account with the Tagging Account number !");	
				write_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, main_acc_pdp, ebufp);
				PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_AAC_PACKAGE, tag_acc_nop, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_tag: Write Flds Input ", write_iflistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, write_iflistp, &write_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_tag: Write Flds Output ", write_oflistp);
				PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
				PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
				
				if (PIN_ERRBUF_IS_ERR(ebufp)) 
				{
					PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
					ret_status = 16;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_15_DESC);
					goto CLEANUP;
				}
		
				/**************************************************************************
				 * Update AAC_PACKAGE of the Tagging Account with the Main Account number
				 **************************************************************************/
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating AAC_PACKAGE of the Tagging Account with the Main Account number !");	
				write_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, tag_acc_pdp, ebufp);
				PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_AAC_PACKAGE, main_acc_nop, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_tag: Write Flds Input ", write_iflistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, write_iflistp, &write_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_tag: Write Flds Output ", write_oflistp);				
				PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
				PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
				
				if (PIN_ERRBUF_IS_ERR(ebufp)) 
				{
					PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
					ret_status = 17;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_16_DESC);
					goto CLEANUP;
				}
				
				/**************************************************************************
				 * If Device ID is avaialable in the Input Flist
				 **************************************************************************/
				devicep = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp);
				if (devicep && strcmp(devicep, "") != 0)
				{
					PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_TYPE, "/device/stb", ebufp);
				}
				else
				{
					devicep = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_VC_ID, 1, ebufp);
					if (devicep && strcmp(devicep, "") != 0)
					{
						PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_TYPE, "/device/vc", ebufp);
					}
				}

				if (devicep && strcmp(devicep, "") != 0)
				{
					/**************************************************************************
					 * Get device details based on the device object
					 **************************************************************************/
					fm_mso_get_device_details(ctxp, i_flistp, devicep, &device_r_flistp, ebufp);
					if (device_r_flistp)
					{
						tmp_flistp = PIN_FLIST_ELEM_GET(device_r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
						serv_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, PIN_FLD_SERVICES,
												PIN_ELEMID_ANY, 1, ebufp);
						if (serv_flistp)
						{
							serv_pdp = PIN_FLIST_FLD_TAKE(serv_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
							offer_namep = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OFFER_NAME, 0, ebufp);
							
							if(!PIN_POID_IS_NULL(serv_pdp))
							{								
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating Scheme Name in CATV Service Object !");	
								write_iflistp = PIN_FLIST_CREATE(ebufp);
								PIN_FLIST_FLD_PUT(write_iflistp, PIN_FLD_POID, serv_pdp, ebufp);
								tmp_flistp = PIN_FLIST_SUBSTR_ADD(write_iflistp, MSO_FLD_CATV_INFO, ebufp);
								PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_OFFER_NAME, offer_namep, ebufp);
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_tag: Write Flds Input ", write_iflistp);
								PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, write_iflistp, &write_oflistp, ebufp);
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_tag: Write Flds Output ", write_oflistp);				
								PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
								PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
								//PIN_FLIST_DESTROY_EX(&serv_flistp, NULL);
								
								if (PIN_ERRBUF_IS_ERR(ebufp)) 
								{
									PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
									ret_status = 18;
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_17_DESC);
									goto CLEANUP;
								}
							}									
						}						
						PIN_FLIST_DESTROY_EX(&device_r_flistp, NULL);			
					}
					else
					{
						ret_status = 19;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_18_DESC);
						goto CLEANUP;
					}
				}
				
				/**************************************************************************
				 * Creating a life cycle event for Tagging Hybrid Accounts - Main Account
				 **************************************************************************/
				PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_ACCOUNT_OBJ, main_acc_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Creating Life Cycle for Tagging Hybrid Accounts - Main Account ");
				fm_mso_create_lifecycle_evt(ctxp, i_flistp, lcevt_ret_flistp, ID_TAG_HYBRID_ACCT, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) 
				{
					ret_status = 20;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_19_DESC);
					goto CLEANUP;
				}

				/**************************************************************************
                                 * Creating a life cycle event for Tagging Hybrid Accounts - Tagged Account
                                 **************************************************************************/
                                PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp); 
				PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_ACCOUNT_OBJ, tag_acc_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Creating Life Cycle for Tagging Hybrid Accounts - Tagged Account ");
                                fm_mso_create_lifecycle_evt(ctxp, i_flistp, lcevt_ret_flistp, ID_TAG_HYBRID_ACCT, ebufp);
                                if (PIN_ERRBUF_IS_ERR(ebufp))
                                {
                                        ret_status = 20;
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_19_DESC);
                                        goto CLEANUP;
                                }

			}
		}		
	}
	else
	{
		ret_status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_MANDFLDS_INVAL_DESC);
		goto CLEANUP;
	}

CLEANUP:

	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Printing the Output Flist for fm_mso_cust_hybrid_accts_tag !");									
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_tag:  Output flist", r_flistp);

	if (ret_status == 0)
	{
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, HYBRID_ACCTS_TAG_SUCCESS_DESC, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_tag: Errors in tagging the Hybrid Accounts ");			
		
		if (ret_status == 1)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_MANDFLDS_INVAL, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_MANDFLDS_INVAL_DESC, ebufp);
		}
		else if (ret_status == 2)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_1, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_1_DESC, ebufp);
		}
		else if (ret_status == 3)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_2, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_2_DESC, ebufp);
		}
		else if (ret_status == 4)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_3, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_3_DESC, ebufp);
		}
		else if (ret_status == 5)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_4, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_4_DESC, ebufp);
		}
		else if (ret_status == 6)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_5, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_5_DESC, ebufp);
		}
		else if (ret_status == 7)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_6, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_6_DESC, ebufp);
		}
		else if (ret_status == 8)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_7, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_7_DESC, ebufp);
		}
		else if (ret_status == 9)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_8, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_8_DESC, ebufp);
		}
		else if (ret_status == 10)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_9, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_9_DESC, ebufp);
		}
		else if (ret_status == 11)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_10, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_10_DESC, ebufp);
		}
		else if (ret_status == 12)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_11, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_11_DESC, ebufp);
		}
		else if (ret_status == 13)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_12, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_12_DESC, ebufp);
		}
		else if (ret_status == 14)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_13, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_13_DESC, ebufp);
		}
		else if (ret_status == 15)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_14, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_14_DESC, ebufp);
		}
		else if (ret_status == 16)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_15, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_15_DESC, ebufp);
		}
		else if (ret_status == 17)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_16, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_16_DESC, ebufp);
		}
		else if (ret_status == 18)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_17, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_17_DESC, ebufp);
		}
		else if (ret_status == 19)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_18, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_18_DESC, ebufp);
		}
		else if (ret_status == 20)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_19, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_19_DESC, ebufp);
		}	
		else if (ret_status == 21)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_20, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_20_DESC, ebufp);
		}	
		else if (ret_status == 22)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_21, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_21_DESC, ebufp);
		}			
	}

	if ((PIN_ERRBUF_IS_ERR(ebufp) || ret_status != 0 ) && t_status == 0 )
	{
		fm_mso_trans_abort(ctxp, main_acc_pdp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"abort transaction");
	}
	else
	{
		fm_mso_trans_commit(ctxp, main_acc_pdp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"commit transaction");
	}
	
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_status, ebufp);	
	*ret_flistpp = r_flistp;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistpp Output flist", *ret_flistpp);
	
	return;
}

/*******************************************************************************
 * fm_mso_cust_hybrid_accts_detag()
 *
 * Inputs: flist with POID, ACCOUNT_NO and ACCOUNT_TAG are mandatory fields 

 * Output: void; ebuf set if errors encountered
 *
 * Description:
 * This function retrieves account poid of the given account number
 * and detags the tagged account number and scheme code. 
 ******************************************************************************/
static void
fm_mso_cust_hybrid_accts_detag(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			flags,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*acnt_infop = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*write_iflistp = NULL;
	pin_flist_t		*write_oflistp = NULL;
	pin_flist_t		*device_r_flistp = NULL;
	pin_flist_t		*serv_flistp = NULL;
	pin_flist_t		*lcevt_ret_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	
	poid_t         		*main_acc_pdp = NULL;
	poid_t         		*tag_acc_pdp = NULL;
	poid_t         		*serv_pdp = NULL;

	char			*main_acc_nop = NULL;
	char			*tag_acc_nop = NULL;
	char			*devicep = NULL;
	char			*offer_namep = NULL;
	char			*existing_main_acc_no = NULL;
	char			*existing_tag_acc_no = NULL;
	
	int32			ret_status = 0;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/*************************************************************
	 * Get the inputs from the Input Flist
	 *************************************************************/
	main_acc_nop = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	tag_acc_nop = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_TAG, 1, ebufp);

	/*************************************************************
	 * Validate the inputs in the Input Flist
	 ************************************************************/
	if ( (main_acc_nop && strlen(main_acc_nop) == 10) && (tag_acc_nop && strlen(tag_acc_nop) == 10) )
	{
		/**********************************************************************************************
		 * Compare if the Main Account and Account to be detagged are same.
		 **********************************************************************************************/
		if(strcmp(main_acc_nop, tag_acc_nop) == 0)
		{
			ret_status = 2;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_DETAG_ERR_1_DESC);
			goto CLEANUP;
		}
		
		/****************************************************************
		 * Get the Main Account details based on Account Number.
		 ****************************************************************/
		fm_mso_get_acnt_from_acntno(ctxp, main_acc_nop, &acnt_infop, ebufp);
		if (acnt_infop)
		{
			main_acc_pdp = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_POID, 0, ebufp);
			existing_tag_acc_no = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_AAC_PACKAGE, 0, ebufp);
			PIN_FLIST_DESTROY_EX(&acnt_infop, ebufp);
		}
		else
		{
			ret_status = 3;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_2_DESC);
			goto CLEANUP;
		}

		/****************************************************************
		 * Get the De-tagging Account details based on Account Number.
		 ****************************************************************/
		fm_mso_get_acnt_from_acntno(ctxp, tag_acc_nop, &acnt_infop, ebufp);
		if (acnt_infop)
		{
			tag_acc_pdp = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_POID, 0, ebufp); 
			existing_main_acc_no = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_AAC_PACKAGE, 0, ebufp);
			PIN_FLIST_DESTROY_EX(&acnt_infop, ebufp);
		}
		else
		{
			ret_status = 4;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_DETAG_ERR_2_DESC);
			goto CLEANUP;
		}
		
		if (existing_tag_acc_no && strcmp(existing_tag_acc_no, "") != 0 && strcmp(existing_tag_acc_no, tag_acc_nop) != 0)
		{
			ret_status = 11;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_DETAG_ERR_8_DESC);
			goto CLEANUP;
		}
		
		if (existing_main_acc_no && strcmp(existing_main_acc_no, "") == 0)
		{
			ret_status = 12;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_DETAG_ERR_9_DESC);
			goto CLEANUP;
		}
		
		if( (existing_main_acc_no && main_acc_nop && strcmp(existing_main_acc_no, main_acc_nop) == 0) && 
			(existing_tag_acc_no && tag_acc_nop && strcmp(existing_tag_acc_no, tag_acc_nop) == 0) )
		{
			if( !PIN_POID_IS_NULL(main_acc_pdp) && !PIN_POID_IS_NULL(tag_acc_pdp) )				
			{
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, main_acc_pdp, ebufp);
				
				/**************************************************************************
				 * Update AAC_PACKAGE of the Main account to NULL to remove tagging
				 **************************************************************************/
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Preparing Write Flds Input for nullifying Tagging Account from Main Account");
				write_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, main_acc_pdp, ebufp);
				PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_AAC_PACKAGE, NULL, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_detag - Account Write Flds Input flist", write_iflistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, write_iflistp, &write_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_detag - Account Write Flds Output flist", write_oflistp);
				PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
				PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
					
				if (PIN_ERRBUF_IS_ERR(ebufp)) 
				{
					PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
					ret_status = 5;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_DETAG_ERR_3_DESC);
					goto CLEANUP;
				}

				/*************************************************************************
				 * Update AAC_PACKAGE of the De-tagging account to NULL to remove tagging
				 *************************************************************************/
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Preparing Write Flds Input for nullifying De-Tagging Account from Main Account");
				write_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID, tag_acc_pdp, ebufp);
				PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_AAC_PACKAGE, NULL, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_detag - Account Write Flds Input flist", write_iflistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, write_iflistp, &write_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_detag - Account Write Flds Output flist", write_oflistp);
				PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
				PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
					
				if (PIN_ERRBUF_IS_ERR(ebufp)) 
				{
					PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
					ret_status = 6;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_DETAG_ERR_4_DESC);
					goto CLEANUP;
				} 
			
				/**************************************************************************
				* If Device ID is avaialable in the Input Flist
				**************************************************************************/
				devicep = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp);
				if (devicep && strcmp(devicep, "") != 0)
				{
					PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_TYPE, "/device/stb", ebufp);
				}
				else
				{
					devicep = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_VC_ID, 1, ebufp);
					if (devicep && strcmp(devicep, "") != 0)
					{
						PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_TYPE, "/device/vc", ebufp);
					}
				}

				if (devicep && strcmp(devicep, "") != 0)
				{
					/**************************************************************************
					 * Get device details based on the device object
					 **************************************************************************/
					fm_mso_get_device_details(ctxp, i_flistp, devicep, &device_r_flistp, ebufp);
					if (device_r_flistp)
					{
						tmp_flistp = PIN_FLIST_ELEM_GET(device_r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
						serv_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
						
						if (serv_flistp)
						{
							serv_pdp = PIN_FLIST_FLD_TAKE(serv_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
							
							if(!PIN_POID_IS_NULL(serv_pdp))
							{					
								/*********************************************************
								* Update Scheme code to NULL to remove tagging
								*********************************************************/				
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
										"Preparing Write Flds Input to update Scheme in the De-Tagging Account Service Object");
								write_iflistp = PIN_FLIST_CREATE(ebufp);
								PIN_FLIST_FLD_PUT(write_iflistp, PIN_FLD_POID, serv_pdp, ebufp);
								tmp_flistp = PIN_FLIST_SUBSTR_ADD(write_iflistp, MSO_FLD_CATV_INFO, ebufp);
								PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_OFFER_NAME, NULL, ebufp);
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
										  "fm_mso_cust_hybrid_accts_detag - Account Write Flds Input flist", write_iflistp);
								PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, write_iflistp, &write_oflistp, ebufp);
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
										"fm_mso_cust_hybrid_accts_detag - Account Write Flds Output flist", write_oflistp);
								PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
								PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
								PIN_FLIST_DESTROY_EX(&serv_flistp, ebufp);
							
								if (PIN_ERRBUF_IS_ERR(ebufp)) 
								{
									PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
									ret_status = 7;
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_DETAG_ERR_5_DESC);
									goto CLEANUP;
								}
							}						
						}			
						PIN_FLIST_DESTROY_EX(&device_r_flistp, ebufp);
					}
					else
					{
						ret_status = 8;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_18_DESC);
						goto CLEANUP;
					}
				}
				
				/*****************************************************************************
				 * Creating a life cycle event for De-Tagging Hybrid Accounts - Main Account
				 *****************************************************************************/
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ, main_acc_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Creating Life Cycle Event for Detagging Hybrid Accounts - Main Account");
				fm_mso_create_lifecycle_evt(ctxp, i_flistp, lcevt_ret_flistp, ID_DETAG_HYBRID_ACCT, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) 
				{
					ret_status = 9;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_DETAG_ERR_6_DESC);
					goto CLEANUP;
				}

				/***************************************************************************
                                 * Creating a life cycle event for Tagging Hybrid Accounts - Tagged Account
                                 ****************************************************************************/
                                PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
                                PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ, tag_acc_pdp, ebufp);
 				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Creating Life Cycle Event for Detagging Hybrid Accounts - Detagged Account");
				fm_mso_create_lifecycle_evt(ctxp, i_flistp, lcevt_ret_flistp, ID_DETAG_HYBRID_ACCT, ebufp);
                                if (PIN_ERRBUF_IS_ERR(ebufp))
                                {
                                        ret_status = 9;
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_DETAG_ERR_6_DESC);
                                        goto CLEANUP;
                                }

			}
		}
		else
		{
			ret_status = 10;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_DETAG_ERR_7_DESC);
			goto CLEANUP;
		}
	}
	else
	{
		ret_status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_DETAG_MANDFLDS_INVAL_DESC);
		goto CLEANUP;
	}
	
CLEANUP:

	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Printing the Output Flist for fm_mso_cust_hybrid_accts_detag !");									
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_detag:  Output flist", r_flistp);
	
	if (ret_status == 0)
	{
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, HYBRID_ACCTS_DETAG_SUCCESS_DESC, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
						"fm_mso_cust_hybrid_accts_detag: Errors in De-Tagging the Hybrid Accounts");

		if (ret_status == 1)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_DETAG_MANDFLDS_INVAL, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_DETAG_MANDFLDS_INVAL_DESC, ebufp);
		}
		else if (ret_status == 2)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_DETAG_ERR_1, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_DETAG_ERR_1_DESC, ebufp);
		}
		else if (ret_status == 3)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_2, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_2_DESC, ebufp);
		}
		else if (ret_status == 4)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_DETAG_ERR_2, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_DETAG_ERR_2_DESC, ebufp);
		}
		else if (ret_status == 5)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_DETAG_ERR_3, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_DETAG_ERR_3_DESC, ebufp);
		}
		else if (ret_status == 6)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_DETAG_ERR_4, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_DETAG_ERR_4_DESC, ebufp);
		}
		else if (ret_status == 7)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_DETAG_ERR_5, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_DETAG_ERR_5_DESC, ebufp);
		}
		else if (ret_status == 8)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_18, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_18_DESC, ebufp);
		}
		else if (ret_status == 9)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_DETAG_ERR_6, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_DETAG_ERR_6_DESC, ebufp);
		}
		else if (ret_status == 10)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_DETAG_ERR_7, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_DETAG_ERR_7_DESC, ebufp);
		}
		else if (ret_status == 11)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_DETAG_ERR_8, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_DETAG_ERR_8_DESC, ebufp);
		}
		else if (ret_status == 12)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_DETAG_ERR_9, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_DETAG_ERR_9_DESC, ebufp);
		}
	}
	
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_status, ebufp);	
	*ret_flistpp = r_flistp;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistpp Output flist", *ret_flistpp);
	
	return;
}

/*******************************************************************************
 * fm_mso_cust_hybrid_accts_retag()
 *
 * Inputs: flist with POID, ACCOUNT_NO and ACCOUNT_TAG are mandatory fields 

 * Output: void; ebuf set if errors encountered
 *
 * Description:
 * This function retrieves account poid of the given account number
 * and detags the old tagged account number and scheme code from the main 
 * account number and retags the new account number and scheme code. 
 ******************************************************************************/
static void
fm_mso_cust_hybrid_accts_retag(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			flags,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*acnt_infop = NULL;
	pin_flist_t		*bill_infop = NULL;
	pin_flist_t		*detagging_iflistp = NULL;
	pin_flist_t		*retagging_iflistp = NULL;	
	pin_flist_t		*detagging_oflistp = NULL;
	pin_flist_t		*retagging_oflistp = NULL;	
	pin_flist_t		*r_flistp = NULL;
	
	poid_t         		*detag_acc_pdp = NULL;
	poid_t         		*retag_acc_pdp   = NULL;
	poid_t         		*main_acc_pdp   = NULL;

	char			*detagact_acc_nop = NULL;
	char			*retagact_acc_nop = NULL;
	char			*main_acc_nop = NULL;
	char			*retagact_acc_tagp = NULL;
	char			*detagact_billinfo_idp = NULL;
	char			*retagact_billinfo_idp = NULL;
	char			*detag_acnt_typep = NULL;	
	char			*retag_acnt_typep = NULL;
	
	int32			*detagact_business_typep = NULL;
	int32			*retagact_business_typep = NULL;	
	int32			ret_status = 0;	
	int32			detag_ret_status = 0;	
	int32			retag_ret_status = 0;
	int32          	 	detag_flags = MSO_HYBRID_DETAGGING;
	int32           	tag_flags = MSO_HYBRID_TAGGING;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/*************************************************************
	 * Get the inputs from the Input Flist
	 *************************************************************/
	detagact_acc_nop = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	retagact_acc_nop = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_TAG, 1, ebufp);

	/*************************************************************
	 * Validate the inputs in the Input Flist
	 ************************************************************/
	if ( (detagact_acc_nop && strlen(detagact_acc_nop) == 10) && (retagact_acc_nop && strlen(retagact_acc_nop) == 10) )
	{
		/**********************************************************************************************
		 * Compare if the Old Account to be Detagged and New Account to be tagged are same.
		 **********************************************************************************************/
		if(strcmp(detagact_acc_nop, retagact_acc_nop) == 0)
		{
			ret_status = 2;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_1_DESC);
			goto CLEANUP;
		}
		
		/****************************************************************
		 * Get the Account details of the Detagging Account.
		 ****************************************************************/		
		fm_mso_get_acnt_from_acntno(ctxp, detagact_acc_nop, &acnt_infop, ebufp);
		if (acnt_infop)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Detagging Account Details ");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Detagging Account Details flist", acnt_infop);
			detag_acc_pdp = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_POID, 0, ebufp);	
			detagact_business_typep = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
			main_acc_nop = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_AAC_PACKAGE, 0, ebufp);
			PIN_FLIST_DESTROY_EX(&acnt_infop, ebufp);
		}
		else
		{
			ret_status = 3;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_2_DESC);
			goto CLEANUP;
		}
		
		/****************************************************************
		 * Get the Account details of the Main Account.
		 ****************************************************************/	
		if(main_acc_nop && strcmp(main_acc_nop, "") == 0)
		{
			ret_status = 19;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_17_DESC);
			goto CLEANUP;
		}
		else if(main_acc_nop && strcmp(main_acc_nop, "") != 0)
		{		
			fm_mso_get_acnt_from_acntno(ctxp, main_acc_nop, &acnt_infop, ebufp);
			if (acnt_infop)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Main Account Details ");
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Main Account Details flist", acnt_infop);
				main_acc_pdp = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_POID, 0, ebufp);					
				PIN_FLIST_DESTROY_EX(&acnt_infop, ebufp);
			}
			else
			{
				ret_status = 17;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_TAG_ERR_2_DESC);
				goto CLEANUP;
			}
		}
		
		if(!PIN_POID_IS_NULL(detag_acc_pdp))
		{
			/****************************************************************
			 * Check the Business Type of Old Detagging Account 
			 ****************************************************************/
			if (detagact_business_typep != NULL && *detagact_business_typep == 0)
			{
				ret_status = 4;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_3_DESC);
				goto CLEANUP;
			}
			
			/****************************************************************
			 * Get the BillInfoID Type of Detagging Account 
			 ****************************************************************/
			fm_mso_get_acct_billinfo(ctxp, detag_acc_pdp, &bill_infop, ebufp);			
			
			if (bill_infop)
			{
				detagact_billinfo_idp = PIN_FLIST_FLD_TAKE(bill_infop, PIN_FLD_BILLINFO_ID, 0, ebufp);
				PIN_FLIST_DESTROY_EX(&bill_infop, ebufp);
			}
			else
			{
				ret_status = 5;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_4_DESC);
				goto CLEANUP;
			}
		}

		/****************************************************************
		 * Get the Account details of the Retagging account
		 ****************************************************************/
		fm_mso_get_acnt_from_acntno(ctxp, retagact_acc_nop, &acnt_infop, ebufp);
		if (acnt_infop)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Retagging Account Details ");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Retagging Account Details flist", acnt_infop);
			retag_acc_pdp = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_POID, 0, ebufp); 
			retagact_business_typep = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_BUSINESS_TYPE, 0, ebufp);				
			retagact_acc_tagp = PIN_FLIST_FLD_TAKE(acnt_infop, PIN_FLD_AAC_PACKAGE, 0, ebufp);
			PIN_FLIST_DESTROY_EX(&acnt_infop, ebufp);
		}
		else
		{
			ret_status = 6;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_5_DESC);
			goto CLEANUP;
		}
		
		if(!PIN_POID_IS_NULL(retag_acc_pdp))
		{
			/****************************************************************
			 * Get the Business Type of Retagging Account 
			 ****************************************************************/
			if (retagact_business_typep != NULL && *retagact_business_typep == 0)
			{
				ret_status = 7;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_6_DESC);
				goto CLEANUP;
			}
			
			/****************************************************************
			 * Get the BillInfoID Type of Retagging Account 
			 ****************************************************************/
			fm_mso_get_acct_billinfo(ctxp, retag_acc_pdp, &bill_infop, ebufp);					
			if (bill_infop)
			{
				retagact_billinfo_idp = PIN_FLIST_FLD_TAKE(bill_infop, PIN_FLD_BILLINFO_ID, 0, ebufp);
				PIN_FLIST_DESTROY_EX(&bill_infop, ebufp);
			}
			else
			{
				ret_status = 8;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_7_DESC);
				goto CLEANUP;
			}
		}
		
		/**********************************************************************
		 * Validate if the Retagging Account already has any account tagged to it.
		 **********************************************************************/				
		if (retagact_acc_tagp && strlen(retagact_acc_tagp) != 0)
		{
			ret_status = 9;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_8_DESC);
			goto CLEANUP;
		}
				
		/***************************************************************************************************
		 * If Detagging Account and Retagging Account do not have the same Bill_Info_ID, then throw error
		 ***************************************************************************************************/	
		if ( detagact_billinfo_idp && retagact_billinfo_idp &&
			 strcmp(detagact_billinfo_idp, retagact_billinfo_idp) != 0 )
		{
			ret_status = 10;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_9_DESC);
			goto CLEANUP;
		}
		
		/***********************************************
		 * Validate the AccountType of Retagging Account
		 ***********************************************/	
		 validate_hybrid_acnt_type(ctxp, retagact_business_typep, retagact_billinfo_idp, &retag_acnt_typep, ebufp);		 
		 if ( PIN_ERRBUF_IS_ERR(ebufp) || (retag_acnt_typep && strcmp(retag_acnt_typep, ACTTYPE_INVALID) == 0) ) 
		 {
			ret_status = 12;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_11_DESC);
			goto CLEANUP;
		 }
		 
		 /***********************************************
		 * Validate the AccountType of Detagging Account
		 ***********************************************/
		 validate_hybrid_acnt_type(ctxp, detagact_business_typep, detagact_billinfo_idp, &detag_acnt_typep, ebufp);
		 if ( PIN_ERRBUF_IS_ERR(ebufp) || (detag_acnt_typep && strcmp(detag_acnt_typep, ACTTYPE_INVALID) == 0) ) 
		 {
			ret_status = 13;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_12_DESC);
			goto CLEANUP;
		 }
		 
		/**************************************************************************************************
		 * If Retagging Account and Detagging Account do not have same Account Types, then throw error
		 **************************************************************************************************/			
		if (retag_acnt_typep && detag_acnt_typep && strcmp(retag_acnt_typep, detag_acnt_typep) != 0)
		{
			ret_status = 14;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_13_DESC);
			goto CLEANUP;
		}
		
		if ( retag_acnt_typep && detag_acnt_typep && 
				 (strcmp(retag_acnt_typep, ACTTYPE_ISP) == 0 && strcmp(detag_acnt_typep, ACTTYPE_ISP) == 0) ||
				 (strcmp(retag_acnt_typep, ACTTYPE_CATV) == 0 && strcmp(detag_acnt_typep, ACTTYPE_CATV) == 0) )
		{			
			/************************************************************************
			 * If the account to be detagged and retagged are available, then check for
			 * Main Account Number / Account POID based on the Tagged Account POID 
			 ************************************************************************/
			if (!PIN_POID_IS_NULL(detag_acc_pdp) && !PIN_POID_IS_NULL(retag_acc_pdp))
			{		
				if ( main_acc_nop && (strcmp(main_acc_nop, "") != 0) )
				{
					PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, main_acc_pdp, ebufp);
			
					/************************************************************************************
					 * Detag the old tagged account number and scheme code from the main account number
					 ************************************************************************************/					
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Preparing Input for De-tagging Old Account from Main Account");
					detagging_iflistp = PIN_FLIST_COPY(i_flistp, ebufp);						
					PIN_FLIST_FLD_DROP(detagging_iflistp, PIN_FLD_ACCOUNT_NO, ebufp);
					PIN_FLIST_FLD_DROP(detagging_iflistp, PIN_FLD_ACCOUNT_TAG, ebufp);
					PIN_FLIST_FLD_DROP(detagging_iflistp, PIN_FLD_FLAGS, ebufp);				
					PIN_FLIST_FLD_SET(detagging_iflistp, PIN_FLD_ACCOUNT_NO, (void *)main_acc_nop, ebufp);
					PIN_FLIST_FLD_SET(detagging_iflistp, PIN_FLD_ACCOUNT_TAG, (void *)detagact_acc_nop, ebufp);
					PIN_FLIST_FLD_SET(detagging_iflistp, PIN_FLD_FLAGS, (void *)&detag_flags, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_retag: Detagging Hybrid Accounts Input ", detagging_iflistp);				 
					fm_mso_cust_hybrid_accts_detag(ctxp, detagging_iflistp, flags, &detagging_oflistp, ebufp);	
					PIN_FLIST_DESTROY_EX(&detagging_iflistp, ebufp);	
					
					if (PIN_ERRBUF_IS_ERR(ebufp)) 
					{
						PIN_FLIST_DESTROY_EX(&detagging_iflistp, NULL);
						ret_status = 15;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_14_DESC);
						goto CLEANUP;
					}
					
					if(detagging_oflistp)
					{
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
									"fm_mso_cust_hybrid_accts_retag: Detagging Hybrid Accounts Output ", detagging_oflistp);
						
						detag_ret_status = *(int32 *)PIN_FLIST_FLD_GET(detagging_oflistp, PIN_FLD_STATUS, 0, ebufp);
						
						if (detag_ret_status != 0)
						{
						   /**************************************************************************************
							* There is an error in the Detagging Hybrid accounts. Return to the main function
							**************************************************************************************/
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_14_DESC);
							r_flistp = detagging_oflistp;
							PIN_ERR_CLEAR_ERR(ebufp);
							return;
						}
						else if (detag_ret_status == 0)
						{
						   /*******************************************************
							* De-Tagging Hybrid Accounts Successful
							*******************************************************/
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "De-Tagging Hybrid Accounts Successful !");
							PIN_FLIST_DESTROY_EX(&detagging_oflistp, ebufp);
						
							/*******************************************************
							* Prepare Input for Re-Tagging of Hybrid Accounts
							*******************************************************/
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Preparing Input for Re-Tagging of Hybrid Accounts !");
							retagging_iflistp = PIN_FLIST_COPY(i_flistp, ebufp);						
							PIN_FLIST_FLD_DROP(retagging_iflistp, PIN_FLD_ACCOUNT_NO, ebufp);
							PIN_FLIST_FLD_DROP(retagging_iflistp, PIN_FLD_ACCOUNT_TAG, ebufp);
							PIN_FLIST_FLD_DROP(retagging_iflistp, PIN_FLD_FLAGS, ebufp);						
							PIN_FLIST_FLD_SET(retagging_iflistp, PIN_FLD_ACCOUNT_NO, (void *)main_acc_nop, ebufp);
							PIN_FLIST_FLD_SET(retagging_iflistp, PIN_FLD_ACCOUNT_TAG, (void *)retagact_acc_nop, ebufp);
							PIN_FLIST_FLD_SET(retagging_iflistp, PIN_FLD_FLAGS, (void *)&tag_flags, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
									"fm_mso_cust_hybrid_accts_retag: Retagging Hybrid Accounts Input ", retagging_iflistp);				
							fm_mso_cust_hybrid_accts_tag(ctxp, retagging_iflistp, flags, &retagging_oflistp, ebufp);
							PIN_FLIST_DESTROY_EX(&retagging_iflistp, ebufp);
							
							if (PIN_ERRBUF_IS_ERR(ebufp)) 
							{
								PIN_FLIST_DESTROY_EX(&retagging_iflistp, NULL);
								ret_status = 16;
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_15_DESC);
								goto CLEANUP;
							}
							
							if(retagging_oflistp)
							{
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
										"fm_mso_cust_hybrid_accts_retag: Retagging Hybrid Accounts Output ", retagging_oflistp);	
								retag_ret_status = *(int32 *)PIN_FLIST_FLD_GET(retagging_oflistp, PIN_FLD_STATUS, 0, ebufp);
						
								if (retag_ret_status == 0)
								{
								   /**************************************************************************************
									* Re-Tagging Hybrid Accounts Successful. Return to main function
									**************************************************************************************/
									PIN_FLIST_FLD_DROP(retagging_oflistp, PIN_FLD_DESCR, ebufp);
									PIN_FLIST_FLD_SET(retagging_oflistp, PIN_FLD_DESCR, HYBRID_ACCTS_RETAG_SUCCESS_DESC, ebufp);
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_ACCTS_RETAG_SUCCESS_DESC);
								}
								
								*ret_flistpp = retagging_oflistp;	
								return;	
							}
						}							
					}				
				}				
			}
			else
			{
				ret_status = 18;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_ERR_16_DESC);
				goto CLEANUP;
			}
		}			
	}
	else
	{
		ret_status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, HYBRID_RETAG_MANDFLDS_INVAL_DESC);
		goto CLEANUP;
	}

CLEANUP:

	if(ret_status != 0)
	{	
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_cust_hybrid_accts_retag: Errors in Re-Tagging Hybrid Accounts");
			
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
	
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Printing the Output Flist for fm_mso_cust_hybrid_accts_retag !");									
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hybrid_accts_retag:  Output flist", r_flistp);
	
		if (ret_status == 1)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_MANDFLDS_INVAL, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_MANDFLDS_INVAL_DESC, ebufp);
		}
		else if (ret_status == 2)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_1, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_1_DESC, ebufp);
		}
		else if (ret_status == 3)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_2, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_2_DESC, ebufp);
		}
		else if (ret_status == 4)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_3, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_3_DESC, ebufp);
		}
		else if (ret_status == 5)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_4, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_4_DESC, ebufp);
		}
		else if (ret_status == 6)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_5, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_5_DESC, ebufp);
		}
		else if (ret_status == 7)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_6, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_6_DESC, ebufp);
		}
		else if (ret_status == 8)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_7, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_7_DESC, ebufp);
		}
		else if (ret_status == 9)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_8, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_8_DESC, ebufp);
		}
		else if (ret_status == 10)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_9, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_9_DESC, ebufp);
		}
		else if (ret_status == 12)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_11, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_11_DESC, ebufp);
		}
		else if (ret_status == 13)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_12, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_12_DESC, ebufp);
		}
		else if (ret_status == 14)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_13, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_13_DESC, ebufp);
		}
		else if (ret_status == 15)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_14, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_14_DESC, ebufp);
		}
		else if (ret_status == 16)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_15, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_15_DESC, ebufp);
		}
		else if (ret_status == 17)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_TAG_ERR_2, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_TAG_ERR_2_DESC, ebufp);
		}
		else if (ret_status == 18)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_16, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_16_DESC, ebufp);
		}
		else if (ret_status == 19)
		{
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, HYBRID_RETAG_ERR_17, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, HYBRID_RETAG_ERR_17_DESC, ebufp);
		}
		
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_status, ebufp);
		*ret_flistpp = r_flistp;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistpp Output flist", *ret_flistpp);
	
	return;
}

