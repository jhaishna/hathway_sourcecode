/**************************************************************************************
 *
 * Copyright (c) 2020 Hathway. All rights reserved.
 *
 * This material is the confidential property of Hathway 
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid DexYP license or sublicense agreement.
 *
 **************************************************************************************/
/***************************************************************************************
 * Maintentance Log:
 *
 * Date: 20203001
 * Author: Tata Consultancy Services 
 * Identifier: CATV Capture Terminate Reasons  
 * Notes: Loading and fetching the Type & Sub Types for the service termination reasons
 ****************************************************************************************/
/*****************************************************************************************
 * File History
 *
 * DATE     |  CHANGE                                           |  Author
 ***********|***************************************************|***************************
 |30/01/2020|Initial vesion                                     | Jyothirmayi Kachiraju
 |**********|***************************************************|***************************
 *******************************************************************************************
 * Opcode Name: MSO_OP_CAPTURE_TERMINATE_REASON 
 * Opcode No: 13210
 * Functionality: Enables Loading and fetching the Type & Sub Types for the service
 *                termination reasons
 * Input Specification:

     --- Loading the Type & Sub Types for the service termination reasons. 
     0 PIN_FLD_POID                  POID [0] 0.0.0.1 / 0 0               // Dummy Poid
     0 PIN_FLD_USERID                POID [0] 0.0.0.1 /account 4924765604 // User ID
     0 PIN_FLD_PROGRAM_NAME           STR [0] "OAP|sushil.sp"             // Program Name
     0 PIN_FLD_TYPE_STR               STR [0] "OPERATIONAL"               // Reason Type
     0 PIN_FLD_VALUE                  STR [0] "High Package Pricing"      // Reason Sub Type
     0 PIN_FLD_FLAGS                  INT [0] 1                           // Flag to load the reason type and subtype
     
     --- Fetching the Type & Sub Types for the service termination reasons. 
     0 PIN_FLD_POID                     POID [0] 0.0.0.1 / 0 0            // Dummy Poid
     0 PIN_FLD_USERID                POID [0] 0.0.0.1 /account 4924765604 // User ID
     0 PIN_FLD_PROGRAM_NAME             STR [0] "OAP|sushil.sp"           // Program Name
     0 PIN_FLD_FLAGS                       INT [0] 2                      // Flag to fetch all the reason types and subtypes
     0 PIN_FLD_PROGRAM_NAME             STR [0] "OAP|sushil.sp"           // Program Name

 * Output Specification: (If Status is 0, the opcode call is successful, other than 0 is failure)

     --- Success Case for Loading the Type & Sub Types for the service termination reasons. :
     
     0 PIN_FLD_POID           POID [0] 0.0.0.1 /mso_cfg_term_reason_master 23486984374 1 
     0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 4924765604 0
     0 PIN_FLD_STATUS         ENUM [0] 0
     0 PIN_FLD_DESCR           STR [0] "Termination Reason Type and Subtype load Successful !"

     --- Success Case for fetching the Type & Sub Types for the service termination reasons. :
     
     0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
     0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 11
     1     PIN_FLD_POID           POID [0] 0.0.0.1 /mso_cfg_term_reason_master 23486994043 1
     1     PIN_FLD_TYPE_STR        STR [0] "TECHNICAL"
     1     MSO_FLD_KEYS          ARRAY [1] allocated 20, used 1
     2         PIN_FLD_VALUE           STR [0] "New VC Allocation"
     1     MSO_FLD_KEYS          ARRAY [0] allocated 20, used 1
     2         PIN_FLD_VALUE           STR [0] "STB Change"
     0 PIN_FLD_RESULTS       ARRAY [1] allocated 20, used 12
     1     PIN_FLD_POID           POID [0] 0.0.0.1 /mso_cfg_term_reason_master 23486996889 2
     1     PIN_FLD_TYPE_STR        STR [0] "OPERATIONAL"
     1     PIN_FLD_USERID         POID [0] 0.0.0.1 /account 4924765604 0
     1     MSO_FLD_KEYS          ARRAY [1] allocated 20, used 1
     2         PIN_FLD_VALUE           STR [0] "Outstanding Issue"
     1     MSO_FLD_KEYS          ARRAY [2] allocated 20, used 1
     2         PIN_FLD_VALUE           STR [0] "Collection Charges"
     1     MSO_FLD_KEYS          ARRAY [0] allocated 20, used 1
     2         PIN_FLD_VALUE           STR [0] "Non-Usage"
     ...............
     0 PIN_FLD_USERID                POID [0] 0.0.0.1 /account 4924765604 
     0 PIN_FLD_STATUS               ENUM [0] 0     
     
     --- Failure Case: 

     0 PIN_FLD_POID           POID [0] 0.0.0.1 / 0 0
     0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 4924765604 0
     0 PIN_FLD_STATUS         ENUM [0] 1
     0 PIN_FLD_ERROR_CODE      STR [0] "95004"
     0 PIN_FLD_ERROR_DESCR     STR [0] "Reason Sub Type already exists for this Reason Type !"
     
 **********************************************************************************************/

#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_cust_load_terminate_reasons.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <pcm.h>
#include <pinlog.h>
#include "cm_fm.h"
#include "pin_errs.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "mso_cust.h"
#include "mso_ops_flds.h"
#include "mso_utils.h"
#include "mso_errs.h"

#define FILE_LOGNAME "fm_mso_cust_load_terminate_reasons.c"

/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_cust_load_terminate_reasons(
     cm_nap_connection_t       *connp,
     int32                     opcode,
     int32                     flags,
     pin_flist_t               *i_flistp,
     pin_flist_t               **o_flistpp,
     pin_errbuf_t              *ebufp);

/************************************************************************
 * This function is used for loading the Type & Sub Types 
 * for the service termination reasons
 ************************************************************************/
static void
fm_mso_cust_load_terminate_reasons(
     pcm_context_t     *ctxp,
     pin_flist_t       *i_flistp,
     pin_flist_t       **ret_flistpp,
     pin_errbuf_t      *ebufp);
     
/************************************************************************
 * This function is used for fetching the Type & Sub Types 
 * for the service termination reasons
 ************************************************************************/
static void
fm_mso_cust_get_terminate_reasons(
     pcm_context_t     *ctxp,
     pin_flist_t       *i_flistp,
     pin_flist_t       **ret_flistpp,
     pin_errbuf_t      *ebufp);

/************************************************************************
 * This function is used for deleting the Type & Sub Types
 * for the service termination reasons
 ************************************************************************/
static void
fm_mso_cust_delete_terminate_reasons(
     pcm_context_t     *ctxp,
     pin_flist_t       *i_flistp,
     pin_flist_t       **ret_flistpp,
     pin_errbuf_t      *ebufp);

/************************************************************************
 * This function is used for deleting the Type & Sub Types
 * for the service termination reasons
 ************************************************************************/
static void
fm_mso_cust_edit_terminate_reasons(
     pcm_context_t     *ctxp,
     pin_flist_t       *i_flistp,
     pin_flist_t       **ret_flistpp,
     pin_errbuf_t      *ebufp);


/************************************************************************
 * This function is used for fetching the Reason Type poid based on
 * the reason type
 ************************************************************************/     
void
fm_mso_cust_get_term_reason_type(
     pcm_context_t          *ctxp,
     poid_t                 *userid,
     char                   *reason_type,
     pin_flist_t            **ret_flistpp,
     pin_errbuf_t           *ebufp);

/************************************************************************
 * This function is trims the leading and trailing spaces
 ************************************************************************/   
void trim(char *str);

/******************************************************************************************
 * Opcode Implementation Starts here
 * Opcode Name: MSO_OP_CAPTURE_TERMINATE_REASON 
 * Opcode No: 13210
 * Functionality: Enables Loading and fetching the Type & Sub Types for the service
 *                termination reasons
 *****************************************************************************************/
void
op_mso_cust_load_terminate_reasons(
     cm_nap_connection_t      *connp,
     int32                    opcode,
     int32                    flags,
     pin_flist_t              *i_flistp,
     pin_flist_t              **o_flistpp,
     pin_errbuf_t             *ebufp)
{
     pcm_context_t            *ctxp = connp->dm_ctx;
     int32                    *status = NULL;
     int32                    *action_flag = NULL ;
     poid_t                   *userid = NULL;     
     pin_flist_t              *ret_flistp = NULL;
     int32                    reason_failure = 1;

     if (PIN_ERRBUF_IS_ERR(ebufp)) 
     {
            return;
     }
     PIN_ERRBUF_CLEAR(ebufp);

     /*************************************************
      * Insanity check.
      ************************************************/
     if (opcode != MSO_OP_CAPTURE_TERMINATE_REASON) 
     {
          pin_set_err(ebufp, PIN_ERRLOC_FM,
               PIN_ERRCLASS_SYSTEM_DETERMINATE,
               PIN_ERR_BAD_OPCODE, 0, 0, opcode);

          PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
               "bad opcode in op_mso_cust_load_terminate_reasons", ebufp);
             return;
     }
	
     PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "starting input");
     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
          "op_mso_cust_load_terminate_reasons input flist", i_flistp);
          
     userid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
     action_flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
     
     if ( !userid || !action_flag )
     {
          ret_flistp = PIN_FLIST_CREATE(ebufp);
          PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
          PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, ret_flistp, PIN_FLD_USERID, ebufp);
          PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &reason_failure, ebufp);
          PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_LOAD_MANDFLDS_MISS, ebufp);
          PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_LOAD_MANDFLDS_MISS_DESC, ebufp);
          *o_flistpp = ret_flistp;
          return;          
     }
          
     if(*(int32 *)action_flag == MSO_TERM_REASONS_LOAD)
     {
          PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Termination reasons loading.");
          fm_mso_cust_load_terminate_reasons(ctxp, i_flistp, o_flistpp, ebufp);
          if (PIN_ERRBUF_IS_ERR(ebufp)) 
          {
               PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "op_mso_cust_load_terminate_reasons Loading : Error", ebufp);
                    
               ret_flistp = PIN_FLIST_CREATE(ebufp);
               PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
               PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, ret_flistp, PIN_FLD_USERID, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &reason_failure, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_LOAD_ERR, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_LOAD_ERR_DESC, ebufp);
               *o_flistpp = ret_flistp;
               return;     
          }
     }
     else if(*(int32 *)action_flag == MSO_TERM_REASONS_FETCH)
     {
          PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Fetching Termination reasons.");
          fm_mso_cust_get_terminate_reasons(ctxp, i_flistp, o_flistpp, ebufp);
          if (PIN_ERRBUF_IS_ERR(ebufp)) 
          {
               PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "op_mso_cust_load_terminate_reasons Get: Error", ebufp);
                                        
               ret_flistp = PIN_FLIST_CREATE(ebufp);
               PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
               PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, ret_flistp, PIN_FLD_USERID, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &reason_failure, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_GET_ERR, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_GET_ERR_DESC, ebufp);
               *o_flistpp = ret_flistp;
               return;     
          }
     }

     else if(*(int32 *)action_flag == MSO_TERM_REASONS_DELETE)
     {
	 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Delete Termination reasons.");
	 fm_mso_cust_delete_terminate_reasons(ctxp, i_flistp, o_flistpp, ebufp);
	 if (PIN_ERRBUF_IS_ERR(ebufp)) 
          {
               PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_load_terminate_reasons Get: Error", ebufp);                                        
               ret_flistp = PIN_FLIST_CREATE(ebufp);
               PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
               PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, ret_flistp, PIN_FLD_USERID, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &reason_failure, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_GET_ERR, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_GET_ERR_DESC, ebufp);
               *o_flistpp = ret_flistp;
               return;     
          }
     }
	
     else if(*(int32 *)action_flag == MSO_TERM_REASONS_EDIT)
     {
	 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Edit Termination reasons.");
	 fm_mso_cust_edit_terminate_reasons(ctxp, i_flistp, o_flistpp, ebufp);
	 if (PIN_ERRBUF_IS_ERR(ebufp)) 
          {
               PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_load_terminate_reasons Get: Error", ebufp);                                        
               ret_flistp = PIN_FLIST_CREATE(ebufp);
               PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
               PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, ret_flistp, PIN_FLD_USERID, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &reason_failure, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_GET_ERR, ebufp);
               PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_GET_ERR_DESC, ebufp);
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
               "op_mso_cust_load_terminate_reasons error", ebufp);
    }

     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
          "op_mso_cust_load_terminate_reasons return flist", *o_flistpp);

    return;
}


/**********************************************************************************************
 * fm_mso_cust_load_terminate_reasons()
 *
 * Inputs: flist with USERID, FLAGS, PIN_FLD_TYPE_STR and PIN_FLD_VALUE are mandatory fields 
 *
 * Output: void; ebuf set if errors encountered
 *
 * Description:
 * This function is used for loading the Type & Sub Types for the service termination reasons
 *********************************************************************************************/
static void
fm_mso_cust_load_terminate_reasons(
     pcm_context_t     *ctxp,
     pin_flist_t       *i_flistp,
     pin_flist_t       **ret_flistpp,
     pin_errbuf_t      *ebufp)
{
     pin_flist_t          *create_iflistp = NULL;
     pin_flist_t          *create_oflistp = NULL;     
     pin_flist_t          *update_iflistp = NULL;
     pin_flist_t          *update_oflistp = NULL;     
     pin_flist_t          *rtype_flistp = NULL;
     pin_flist_t          *subtypes_flistp = NULL;
     pin_flist_t          *tmp_flistp = NULL;
     pin_flist_t          *r_flistp = NULL;

     poid_t               *userid = NULL;
     poid_t               *rtype_poid = NULL;
     
     char                 *reason_type = NULL;
     char                 *reason_subtype = NULL;
     char                 *existing_res_subtype = NULL;
     char                 *program_name = NULL;
     
     int32                ret_failure = 1;     
     int32                ret_success = 0;
     int32                subtype_exists = 0;
     int                  rec_id =0;

     pin_cookie_t         cookie = NULL;

     if (PIN_ERRBUF_IS_ERR(ebufp))
          return;
     PIN_ERRBUF_CLEAR(ebufp);
     
     /*************************************************************
      * Get the inputs from the Input Flist
      *************************************************************/
     userid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
     reason_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE_STR, 1, ebufp);
     reason_subtype = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_VALUE, 1, ebufp);
     program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
     
     r_flistp = PIN_FLIST_CREATE(ebufp);
     PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, r_flistp, PIN_FLD_USERID, ebufp);
     
     /*************************************************************
      * Validate the inputs in the Input Flist
      ************************************************************/
     if ( (reason_type && strlen(reason_type) == 0) && 
           (reason_subtype && strlen(reason_subtype) == 0))
     {
          PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_LOAD_ERR_1_DESC);     
          PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
          PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
          PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_LOAD_ERR_1, ebufp);
          PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_LOAD_ERR_1_DESC, ebufp);
     }
     else
     {
          trim(reason_type);
          trim(reason_subtype);

          fm_mso_cust_get_term_reason_type(ctxp, userid, reason_type, &rtype_flistp, ebufp);
          
          if (rtype_flistp)
          {
               rtype_poid = PIN_FLIST_FLD_GET(rtype_flistp, PIN_FLD_POID, 1, ebufp);
               tmp_flistp = PIN_FLIST_ELEM_TAKE(rtype_flistp, MSO_FLD_KEYS, 0, 1, ebufp);
               
               while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (rtype_flistp, MSO_FLD_KEYS, &rec_id, 1, &cookie, ebufp)) != NULL ) 
               {
                    existing_res_subtype = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_VALUE, 0, ebufp);
                    trim(existing_res_subtype);

                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "existing reason sub type >> ");
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, existing_res_subtype);
                    
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "new reason sub type >> ");
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, reason_subtype);
                    
                    if ( existing_res_subtype && strcmp(reason_subtype, existing_res_subtype) == 0)
                    {
                         subtype_exists = 1;
                         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_LOAD_ERR_2_DESC);
                         PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
                         PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);                         
                         PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_LOAD_ERR_2, ebufp);
                         PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_LOAD_ERR_2_DESC, ebufp);
                    }
               }
                                        
               if ( rtype_poid && !(PIN_POID_IS_NULL(rtype_poid)) && subtype_exists == 0 )
               {
                    update_iflistp = PIN_FLIST_CREATE(ebufp);
                    PIN_FLIST_FLD_SET(update_iflistp, PIN_FLD_POID, rtype_poid, ebufp);
                    PIN_FLIST_FLD_SET(update_iflistp, PIN_FLD_USERID, userid, ebufp);
                    PIN_FLIST_FLD_SET(update_iflistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);                    
                    subtypes_flistp = PIN_FLIST_ELEM_ADD(update_iflistp, MSO_FLD_KEYS, PIN_ELEMID_ASSIGN, ebufp);
                    PIN_FLIST_FLD_SET(subtypes_flistp, PIN_FLD_VALUE, reason_subtype, ebufp);
                    
                    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_load_terminate_reasons Update input ", update_iflistp);
                    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_ADD_ENTRY, update_iflistp, &update_oflistp, ebufp);
                    PIN_FLIST_DESTROY_EX(&update_iflistp, NULL);

                    if (PIN_ERRBUF_IS_ERR(ebufp) || !update_oflistp)
                    {
                         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_LOAD_ERR_3_DESC);     
                         PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
                         PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
                         PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_LOAD_ERR_3, ebufp);
                         PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_LOAD_ERR_3_DESC, ebufp);                    
                    }
                    else
                    {
                         rtype_poid = PIN_FLIST_FLD_GET(update_oflistp, PIN_FLD_POID, 1, ebufp);
                         if ( rtype_poid && !(PIN_POID_IS_NULL(rtype_poid) ) )
                         {                         
                              PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, rtype_poid, ebufp);
                         }
                         PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_success, ebufp);
                         PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, TERM_REASONS_LOAD_SUCCESS_MSG, ebufp);                    
                    }
               
                    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_load_terminate_reasons Update output", update_oflistp);
                    PIN_FLIST_DESTROY_EX(&update_oflistp, NULL);
               }
               
               PIN_FLIST_DESTROY_EX(&rtype_flistp, NULL);
          }               
          else
          {     
               create_iflistp = PIN_FLIST_CREATE(ebufp);
               
               PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_POID, 
                                                  PIN_POID_CREATE(PIN_POID_GET_DB(userid), "/mso_cfg_term_reason_master", (int64)-1, ebufp), ebufp);
               PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_TYPE_STR, reason_type, ebufp);
               PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_USERID, userid, ebufp);
               PIN_FLIST_FLD_SET(create_iflistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);     
               subtypes_flistp = PIN_FLIST_ELEM_ADD(create_iflistp, MSO_FLD_KEYS, 0, ebufp);
               PIN_FLIST_FLD_SET(subtypes_flistp, PIN_FLD_VALUE, reason_subtype, ebufp);
               
               PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_load_terminate_reasons Create input ", create_iflistp);
               PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_iflistp, &create_oflistp, ebufp);
               PIN_FLIST_DESTROY_EX(&create_iflistp, NULL);

               if (PIN_ERRBUF_IS_ERR(ebufp) || !create_oflistp)
               {
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_LOAD_ERR_4_DESC);     
                    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);                    
                    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
                    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_LOAD_ERR_4, ebufp);
                    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_LOAD_ERR_4_DESC, ebufp);                    
               }
               else
               {
                    rtype_poid = PIN_FLIST_FLD_GET(create_oflistp, PIN_FLD_POID, 1, ebufp);
                    if ( rtype_poid && !(PIN_POID_IS_NULL(rtype_poid)) )
                    {                         
                         PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, rtype_poid, ebufp);
                    }
                    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_success, ebufp);
                    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, TERM_REASONS_LOAD_SUCCESS_MSG, ebufp);                    
               }
                              
               PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_load_terminate_reasons Create output", create_oflistp);
               PIN_FLIST_DESTROY_EX(&create_oflistp, NULL);
          }
     }
     
     if (r_flistp)
     {
          *ret_flistpp = r_flistp;
     }
     
     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_load_terminate_reasons Output flist", *ret_flistpp);
     
     return;
}

/*******************************************************************************
 * fm_mso_cust_get_terminate_reasons()
 *
 * Inputs: flist with USERID, FLAGS are mandatory fields 
 *
 * Output: void; ebuf set if errors encountered
 *
 * Description:
 * This function is used to get the service termination reason details
 ******************************************************************************/
static void
fm_mso_cust_get_terminate_reasons(
     pcm_context_t          *ctxp,
     pin_flist_t            *i_flistp,
     pin_flist_t            **ret_flistpp,
     pin_errbuf_t           *ebufp)
{
     pin_flist_t          *r_flistp = NULL;
     pin_flist_t          *search_iflistp = NULL;
     pin_flist_t          *search_oflistp = NULL;     
     pin_flist_t          *args_flistp = NULL;
     pin_flist_t          *result_flistp = NULL;
     pin_flist_t          *types_flistp = NULL;
     pin_flist_t          *subtypes_flistp = NULL;
     
     poid_t               *srch_pdp = NULL;
     poid_t               *rtype_poid = NULL;
     poid_t               *userid = NULL;

     int32               srch_flags = 768;
     int32               ret_failure = 1;     
     int32               ret_success = 0;     

     char                *srch_template =  " select X from /mso_cfg_term_reason_master where F1.type = V1 ";
     
     if (PIN_ERRBUF_IS_ERR(ebufp))
          return;
     PIN_ERRBUF_CLEAR(ebufp);
     
     /*****************************************************************
      * Get the inputs from the Input Flist
      *****************************************************************/
     userid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);

     search_iflistp = PIN_FLIST_CREATE(ebufp);
     PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, PIN_POID_CREATE(PIN_POID_GET_DB(userid), "/search", (int64)-1, ebufp), ebufp);
     PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
     PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_TEMPLATE, srch_template, ebufp);
     
     rtype_poid = PIN_POID_CREATE(PIN_POID_GET_DB(userid), "/mso_cfg_term_reason_master", (int64)-1, ebufp);
     args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 1, ebufp);
     PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, rtype_poid, ebufp);

     result_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
     PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
     PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_TYPE_STR, NULL, ebufp);
     subtypes_flistp = PIN_FLIST_ELEM_ADD(result_flistp, MSO_FLD_KEYS, PIN_ELEMID_ANY, ebufp);
     PIN_FLIST_FLD_SET(subtypes_flistp, PIN_FLD_VALUE, NULL, ebufp);
     
     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_terminate_reasons search input ", search_iflistp);
     PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_iflistp, &search_oflistp, ebufp);
     PIN_FLIST_DESTROY_EX(&search_iflistp, NULL);
     
     if (PIN_ERRBUF_IS_ERR(ebufp) || !search_oflistp)
     {
          r_flistp = PIN_FLIST_CREATE(ebufp);
          PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
          PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, r_flistp, PIN_FLD_USERID, ebufp);
          PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_LOAD_ERR_5_DESC);          
          PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
          PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_LOAD_ERR_5, ebufp);
          PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_LOAD_ERR_5_DESC, ebufp);                    
     }
     else
     {
          r_flistp = PIN_FLIST_COPY(search_oflistp, ebufp);
          PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_USERID, i_flistp, PIN_FLD_USERID, ebufp);
          PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_success, ebufp);               
     }

     PIN_FLIST_DESTROY_EX(&search_oflistp, NULL);
     
     if (r_flistp)
     {
          *ret_flistpp = r_flistp;
     }
     
     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_terminate_reasons Output flist", *ret_flistpp);
     
     return;
}
 
/*******************************************************************************
 * fm_mso_cust_get_term_reason_type()
 *
 * Inputs: flist with ctxp, PIN_FLD_TYPE_STR, ebufp as mandatory fields 

 * Output: void; ebuf set if errors encountered
 *
 * Description:
 * This function retrieves flist with the poid of the existing reason type
 ******************************************************************************/
void
fm_mso_cust_get_term_reason_type(
     pcm_context_t          *ctxp,
     poid_t                 *userid,
     char                   *reason_type,
     pin_flist_t            **ret_flistpp,
     pin_errbuf_t           *ebufp)
{
     pin_flist_t           *search_iflistp = NULL;
     pin_flist_t           *search_oflistp = NULL;     
     pin_flist_t           *args_flistp = NULL;
     pin_flist_t           *result_flistp = NULL;
     pin_flist_t           *subtypes_flistp = NULL;
     
     poid_t                *srch_pdp = NULL;

     int32                 srch_flags = 768;

     char                  *srch_template =  " select X from /mso_cfg_term_reason_master where F1 = V1 ";

     if (PIN_ERRBUF_IS_ERR(ebufp))
          return;
     PIN_ERRBUF_CLEAR(ebufp); 
 
     search_iflistp = PIN_FLIST_CREATE(ebufp);
     PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_POID, PIN_POID_CREATE(PIN_POID_GET_DB(userid), "/search", (int64)-1, ebufp), ebufp);
     PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
     PIN_FLIST_FLD_SET(search_iflistp, PIN_FLD_TEMPLATE, srch_template, ebufp);
     
     args_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_ARGS, 1, ebufp);
     PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE_STR, reason_type, ebufp);

     result_flistp = PIN_FLIST_ELEM_ADD(search_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
     PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
     subtypes_flistp = PIN_FLIST_ELEM_ADD(result_flistp, MSO_FLD_KEYS, PIN_ELEMID_ANY, ebufp);
     PIN_FLIST_FLD_SET(subtypes_flistp, PIN_FLD_VALUE, NULL, ebufp);
     
     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_term_reason_type search input ", search_iflistp);
     PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_iflistp, &search_oflistp, ebufp);
     PIN_FLIST_DESTROY_EX(&search_iflistp, NULL);

     if (PIN_ERRBUF_IS_ERR(ebufp))
     {
          PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Termination Reason Type Search", ebufp);
          return;
     }
     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_term_reason_type search output", search_oflistp);

     if (search_oflistp)
     {
          result_flistp = PIN_FLIST_ELEM_TAKE(search_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
          
          if(result_flistp)
          {
               *ret_flistpp = result_flistp;     
          }                    
     }
     
     PIN_FLIST_DESTROY_EX(&search_oflistp, NULL);

     return;
}

/*******************************************************************************
 * trim()
 *
 * Inputs: String
 *
 * Output: void; ebuf set if errors encountered
 *
 * Description:
 * This function is used to trim leading and trailing spaces
 ******************************************************************************/
void trim(char *str)
{
    int index = 0;
	int i = 0;

    /*
     * Trim leading white spaces
     */
    while(str[index] == ' ' || str[index] == '\t' || str[index] == '\n')
    {
        index++;
    }

    /* Shift all trailing characters to its left */
    while(str[i + index] != '\0')
    {
        str[i] = str[i + index];
        i++;
    }
    str[i] = '\0'; // Terminate string with NULL


    /*
     * Trim trailing white spaces
     */
    i = 0;
    index = -1;
    while(str[i] != '\0')
    {
        if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
        {
            index = i;
        }

        i++;
    }

    /* Mark the next character to last non white space character as NULL */
    str[index + 1] = '\0';
}



/**********************************************************************************************
 * fm_mso_cust_delete_terminate_reasons()
 *
 * Inputs: flist with USERID, FLAGS, PIN_FLD_TYPE_STR and PIN_FLD_VALUE are mandatory fields 
 *
 * Output: void; ebuf set if errors encountered
 *
 * Description:
 * This function is used for deleting the Type & Sub Types for the service termination reasons
 *********************************************************************************************/
static void
fm_mso_cust_delete_terminate_reasons(
     pcm_context_t     *ctxp,
     pin_flist_t       *i_flistp,
     pin_flist_t       **ret_flistpp,
     pin_errbuf_t      *ebufp)
{
     pin_flist_t          *create_iflistp = NULL;
     pin_flist_t          *create_oflistp = NULL;     
     pin_flist_t          *update_iflistp = NULL;
     pin_flist_t          *update_oflistp = NULL;     
     pin_flist_t          *delete_iflistp = NULL;     
     pin_flist_t          *delete_rflistp = NULL;     
     pin_flist_t          *rtype_flistp = NULL;
     pin_flist_t          *subtypes_flistp = NULL;
     pin_flist_t          *tmp_flistp = NULL;
     pin_flist_t          *r_flistp = NULL;

     poid_t               *userid = NULL;
     poid_t               *rtype_poid = NULL;
     
     char                 *reason_type = NULL;
     char                 *reason_subtype = NULL;
     char                 *existing_res_subtype = NULL;
     char                 *program_name = NULL;
     
     int32                ret_failure = 1;     
     int32                ret_success = 0;
     int32                subtype_exists = 0;
     int32                elem_id = 0;
     pin_cookie_t         cookie = NULL;

     if (PIN_ERRBUF_IS_ERR(ebufp))
          return;
     PIN_ERRBUF_CLEAR(ebufp);
     
     /*************************************************************
      * Get the inputs from the Input Flist
      *************************************************************/
     userid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
     reason_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE_STR, 1, ebufp);
     reason_subtype = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_VALUE, 1, ebufp);
     program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
     
     r_flistp = PIN_FLIST_CREATE(ebufp);
     PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, r_flistp, PIN_FLD_USERID, ebufp);

	/*************************************************************
      	* Validate the inputs in the Input Flist 
      	************************************************************/
     if ( (reason_type && strlen(reason_type) == 0) && (reason_subtype && strlen(reason_subtype) == 0)) /*reason_type and reason_subtype should not be empty*/
     {
          PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_LOAD_ERR_1_DESC);     
          PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
          PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
          PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_LOAD_ERR_1, ebufp);
          PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_LOAD_ERR_1_DESC, ebufp);
     }
     else if (reason_type && strlen(reason_type) != 0) /*If reason type is sent in input*/
     {
          trim(reason_type);
     
          fm_mso_cust_get_term_reason_type(ctxp, userid, reason_type, &rtype_flistp, ebufp);
          
          if (rtype_flistp) /*Reason type object exist in table*/
          {
				rtype_poid = PIN_FLIST_FLD_GET(rtype_flistp, PIN_FLD_POID, 1, ebufp);
				if (reason_subtype && strlen(reason_subtype) != 0) /*If subtype also exist in input*/
				{
				    trim(reason_subtype);

					while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (rtype_flistp, MSO_FLD_KEYS, &elem_id, 1, &cookie, ebufp)) != NULL ) 
					{
						existing_res_subtype = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_VALUE, 0, ebufp);
						trim(existing_res_subtype);

						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "existing reason sub type >> ");
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, existing_res_subtype);
                    
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "input reason sub type >> ");
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, reason_subtype);
                    
						if ( existing_res_subtype && strcmp(reason_subtype, existing_res_subtype) == 0)
						{
							subtype_exists =1;
							update_iflistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(update_iflistp, PIN_FLD_POID, rtype_poid, ebufp);
							                  
							subtypes_flistp = PIN_FLIST_ELEM_ADD(update_iflistp, MSO_FLD_KEYS, elem_id, ebufp);
							PIN_FLIST_FLD_SET(subtypes_flistp, PIN_FLD_VALUE, reason_subtype, ebufp);
						
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_delete_terminate_reasons Update input", update_iflistp);
							PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, update_iflistp, &update_oflistp, ebufp);
							PIN_FLIST_DESTROY_EX(&update_iflistp, NULL);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_delete_terminate_reasons Update output", update_oflistp);
					
							if (PIN_ERRBUF_IS_ERR(ebufp))
							{	
								 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_DELETE_ERR_3_DESC);     
								 PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
								 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
								 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_DELETE_ERR_3, ebufp);
								 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_DELETE_ERR_3_DESC, ebufp);                    
							}
							else
							{
								 rtype_poid = PIN_FLIST_FLD_GET(update_oflistp, PIN_FLD_POID, 1, ebufp);
								 if ( rtype_poid && !(PIN_POID_IS_NULL(rtype_poid) ) )
								 {                         
									  PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, rtype_poid, ebufp);
								 }
								 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_success, ebufp);
								 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, TERM_REASONS_DELETE_SUCCESS_MSG, ebufp);                    
							}
						PIN_FLIST_DESTROY_EX(&update_oflistp, NULL);
						break;
						}
					}
					if(subtype_exists !=1) /*If subtype doesn't exists in table*/
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_DELETE_ERR_2_DESC);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);                         
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_DELETE_ERR_2, ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_DELETE_ERR_2_DESC, ebufp);
					}			   
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Deleting reasontype start!");
					delete_iflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(delete_iflistp, PIN_FLD_POID, rtype_poid, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Delete reason_type input flist", delete_iflistp);
					PCM_OP(ctxp, PCM_OP_DELETE_OBJ, 0, delete_iflistp, &delete_rflistp, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Delete reason_type output flist", delete_rflistp);

					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
							 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_DELETE_ERR_3_DESC);
							 PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
							 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
							 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_DELETE_ERR_3, ebufp);
							 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_DELETE_ERR_3_DESC, ebufp);
					}
					else
					{
							 rtype_poid = PIN_FLIST_FLD_GET(delete_rflistp, PIN_FLD_POID, 1, ebufp);
							 if ( rtype_poid && !(PIN_POID_IS_NULL(rtype_poid) ) )
							 {
								  PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, rtype_poid, ebufp);
							 }
							 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_success, ebufp);
							 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, TERM_REASONS_DELETE_SUCCESS_MSG, ebufp);
					}

					PIN_FLIST_DESTROY_EX(&delete_iflistp, NULL);
					PIN_FLIST_DESTROY_EX(&delete_rflistp, NULL);	
				}
			}		
			else /*Reason type object doesnt exist in table*/
			{     
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_DELETE_ERR_1_DESC);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);                         
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_DELETE_ERR_1, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_DELETE_ERR_1_DESC, ebufp);
			}
			PIN_FLIST_DESTROY_EX(&rtype_flistp, NULL);
     }
     
     if (r_flistp)
     {
          *ret_flistpp = r_flistp;
     }
     
     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_delete_terminate_reasons Output flist", *ret_flistpp);
     
     return;
}




/**********************************************************************************************
 * fm_mso_cust_edit_terminate_reasons()
 *
 * Inputs: flist with USERID, FLAGS, PIN_FLD_TYPE_STR and PIN_FLD_VALUE are mandatory fields 
 *
 * Output: void; ebuf set if errors encountered
 *
 * Description:
 * This function is used for editing the Type & Sub Types for the service termination reasons
 *********************************************************************************************/
static void
fm_mso_cust_edit_terminate_reasons(
     pcm_context_t     *ctxp,
     pin_flist_t       *i_flistp,
     pin_flist_t       **ret_flistpp,
     pin_errbuf_t      *ebufp)
{
     pin_flist_t          *create_iflistp = NULL;
     pin_flist_t          *create_oflistp = NULL;     
     pin_flist_t          *update_iflistp = NULL;
     pin_flist_t          *update_oflistp = NULL;     
     pin_flist_t          *rtype_flistp = NULL;
     pin_flist_t          *subtypes_flistp = NULL;
     pin_flist_t          *tmp_flistp = NULL;
     pin_flist_t          *r_flistp = NULL;
     pin_flist_t	  *read_reason_iflistp = NULL;
     pin_flist_t	  *read_reason_oflistp = NULL;

     poid_t               *userid = NULL;
     poid_t               *rtype_poid = NULL;
     poid_t		  *input_poid = NULL;
     
     char                 *reason_type = NULL;
     char                 *reason_subtype = NULL;
     char                 *existing_res_type = NULL;
     char		  *existing_res_subtype = NULL;
     char                 *program_name = NULL;
     char		  *old_reason_subtype = NULL;
     
     int32                ret_failure = 1;     
     int32                ret_success = 0;
     int32                subtype_exists = 0;
     int                  rec_id =0;
     int		  elem_id=0;

     pin_cookie_t         cookie = NULL;

     if (PIN_ERRBUF_IS_ERR(ebufp))
          return;
     PIN_ERRBUF_CLEAR(ebufp);

      /*************************************************************
      * Get the inputs from the Input Flist
      *************************************************************/
     reason_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE_STR, 1, ebufp);
     reason_subtype = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_VALUE, 1, ebufp);
     old_reason_subtype = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OLD_VALUE, 1, ebufp);
     input_poid = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_POID,1,ebufp);

     r_flistp = PIN_FLIST_CREATE(ebufp);
     PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, r_flistp, PIN_FLD_USERID, ebufp);

     /*************************************************************
      * Validate the inputs in the Input Flist
      ************************************************************/

     if ( (reason_type && strlen(reason_type) == 0) && (reason_subtype && strlen(reason_subtype) == 0) )  /*reason_type and reason_subtype should not be empty*/
     {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_LOAD_ERR_1_DESC);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_LOAD_ERR_1, ebufp);
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_LOAD_ERR_1_DESC, ebufp);
     }
     else
     {
        read_reason_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, read_reason_iflistp, PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_edit_terminate_reasons read flds input list", read_reason_iflistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_reason_iflistp, &read_reason_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_edit_terminate_reasons read flds output list", read_reason_oflistp);

        if (read_reason_oflistp) /*POID record exist*/
        {
                        trim(reason_subtype);

                        while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (read_reason_oflistp, MSO_FLD_KEYS, &elem_id, 1, &cookie, ebufp)) != NULL )
                        {
                                existing_res_subtype = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_VALUE, 0, ebufp);
                                trim(existing_res_subtype);

                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "existing reason sub type >> ");
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, existing_res_subtype);

                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "input old reason sub type >> ");
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, old_reason_subtype);

                                if ( existing_res_subtype && strcmp(old_reason_subtype, existing_res_subtype) == 0)
                                {
                                        subtype_exists =1;
                                        update_iflistp = PIN_FLIST_CREATE(ebufp);
                                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, update_iflistp, PIN_FLD_POID, ebufp);
                                        subtypes_flistp = PIN_FLIST_ELEM_ADD(update_iflistp, MSO_FLD_KEYS, elem_id, ebufp);
                                        PIN_FLIST_FLD_SET(subtypes_flistp, PIN_FLD_VALUE, reason_subtype, ebufp);

                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_edit_terminate_reasons Update input", update_iflistp);
                                        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_iflistp, &update_oflistp, ebufp);
                                        PIN_FLIST_DESTROY_EX(&update_iflistp, NULL);
                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_edit_terminate_reasons Update output", update_oflistp);

                                        if (PIN_ERRBUF_IS_ERR(ebufp))
                                        {
                                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_EDIT_ERR_1_DESC);
                                                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_EDIT_ERR_1, ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_EDIT_ERR_1_DESC, ebufp);
                                        }
                                        else
                                        {
                                                rtype_poid = PIN_FLIST_FLD_GET(update_oflistp, PIN_FLD_POID, 1, ebufp);
                                                if ( rtype_poid && !(PIN_POID_IS_NULL(rtype_poid) ) )
                                                {
                                                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, rtype_poid, ebufp);
                                                }
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_success, ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, TERM_REASONS_EDIT_SUCCESS_MSG, ebufp);
                                        }
                                        PIN_FLIST_DESTROY_EX(&update_oflistp, NULL);
                                        PIN_FLIST_DESTROY_EX(&update_iflistp, NULL);
                                        break;
                                }
                        }
                        if(subtype_exists !=1) /*If subtype doesn't exists in table*/
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_DELETE_ERR_2_DESC);
                                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_DELETE_ERR_2, ebufp);
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_DELETE_ERR_2_DESC, ebufp);
                        }
                   
                        trim(reason_type);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Editing reasontype start!");
                        existing_res_type = PIN_FLIST_FLD_GET(read_reason_oflistp, PIN_FLD_TYPE_STR, 0, ebufp);
                        trim(existing_res_type);
                        if ( existing_res_type && strcmp(reason_type, existing_res_type) != 0)
                        {
                                update_iflistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, update_iflistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TYPE_STR, update_iflistp, PIN_FLD_TYPE_STR, ebufp);
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_edit_terminate_reasons Update input ", update_iflistp);
                                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_ADD_ENTRY, update_iflistp, &update_oflistp, ebufp);
                                PIN_FLIST_DESTROY_EX(&update_iflistp, NULL);

                                if (PIN_ERRBUF_IS_ERR(ebufp) || !update_oflistp)
                                {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_LOAD_ERR_3_DESC);
                                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
                                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
                                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_LOAD_ERR_3, ebufp);
                                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_LOAD_ERR_3_DESC, ebufp);
                                }
                                else
                                {
                                        rtype_poid = PIN_FLIST_FLD_GET(update_oflistp, PIN_FLD_POID, 1, ebufp);
                                        if ( rtype_poid && !(PIN_POID_IS_NULL(rtype_poid) ) )
                                        {
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, rtype_poid, ebufp);
                                        }
                                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_success, ebufp);
                                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, TERM_REASONS_EDIT_SUCCESS_MSG, ebufp);
                                }
                                PIN_FLIST_DESTROY_EX(&update_oflistp, NULL);
                                PIN_FLIST_DESTROY_EX(&update_iflistp, NULL);
                        }
            
        }
        else /* POID object doesnt exist in table*/
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, TERM_REASONS_DELETE_ERR_1_DESC);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &ret_failure, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, TERM_REASONS_DELETE_ERR_1, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, TERM_REASONS_DELETE_ERR_1_DESC, ebufp);
        }
        PIN_FLIST_DESTROY_EX(&read_reason_iflistp, NULL);
        PIN_FLIST_DESTROY_EX(&read_reason_oflistp, NULL);
    }

     if (r_flistp)
     {
          *ret_flistpp = r_flistp;
     }

     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_edit_terminate_reasons Output flist", *ret_flistpp);

     return;
}
