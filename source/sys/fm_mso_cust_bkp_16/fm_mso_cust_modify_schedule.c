/*******************************************************************
 * File:        fm_mso_cust_modify_schedule.c
 * Opcode:      MSO_OP_CUST_CREATE_SCHEDULE
 * Owner:       Joe De los Reyes
 * Created:     03-DEC-2013
 * Description:
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist
 * Modified by Sisir to add hold & unhold action on 20-May-2015

********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_modify_schedule.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pcm_ops.h"
#include "pin_flds.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_lifecycle_id.h"

#define READONLY 0
#define READWRITE 1
#define LOCK_OBJ 2

#define LOCAL_TRANS_OPEN_SUCCESS 0
#define LOCAL_TRANS_OPEN_FAIL 1


#define NAMEINFO_BILLING 1
#define NAMEINFO_INSTALLATION 2

#define ACCOUNT_MODEL_UNKNOWN 0
#define ACCOUNT_MODEL_RES 1
#define ACCOUNT_MODEL_CORP 2
#define ACCOUNT_MODEL_MDU 3

#define MSO_ACTION_ADD_PLAN 0 
#define MSO_ACTION_CANCEL_PLAN 1
#define MSO_ACTION_CHANGE_PLAN 2
#define MSO_ACTION_ACTIVATE_CUSTOMER 3
#define MSO_ACTION_SUSPEND_SERVICE 4
#define MSO_ACTION_REACTIVATE_SERVICE 5
#define MSO_ACTION_BMAIL 6
#define MSO_ACTION_OSD 7
#define MSO_ACTION_MODIFY_SCHEDULE 8 
#define MSO_ACTION_DELETE_SCHEDULE 9 
#define MSO_ACTION_TOPUP 10
#define MSO_ACTION_RENEW 11
#define MSO_ACTION_CREDIT_NOTE 12

// Added below action
#define MSO_ACTION_HOLD 13
#define MSO_ACTION_UNHOLD 14

/**************************************
* External Functions start
***************************************/
extern int32
fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

extern void  
fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

/**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/
static int 
fm_mso_modify_scheduler(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
        int                     action_code,
        int                     opcode,
	pin_errbuf_t		*ebufp);
	
/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_CREATE_SCHEDULE 
 *
 * This policy provides a hooks for enhancing OP_ACT_CUST_CREATE_SCHEDULE
 * before calling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_modify_schedule(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_modify_schedule(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_CREATE_SCHEDULE  
 *******************************************************************/
void 
op_mso_cust_modify_schedule(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	int			local = LOCAL_TRANS_OPEN_SUCCESS;
	int			*status = NULL;
	poid_t			*inp_pdp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_MODIFY_SCHEDULE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_modify_schedule error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_modify_schedule input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_cust_modify_schedule()", ebufp);
		return;
	}
	fm_mso_cust_modify_schedule(ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_modify_schedule error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_modify_schedule output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_modify_schedule(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*o_flistp = NULL;
	pin_flist_t		*update_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*lc_in_flist = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*account_obj = NULL;

	int32			 sched_job_success = 0;
	int32			 sched_job_failure = 1;
	int32			 caller = MSO_OP_CUST_MODIFY_SCHEDULE;
	int64			 db = -1;
	int			 csr_access = 0;
	int			 acct_update = 0;
	int			action_code = 0;
	char			 mesg[200];
        int32			*status = NULL;	
        int32			*new_state =NULL;
        time_t                  *when_t = 0;
        poid_t 			*sch_pdp =NULL;

	void			*vp = NULL;
       

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_schedule input flist", i_flistp);	
	memset(mesg,'\0',sizeof(mesg));	
	sch_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        status  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 0, ebufp);

        if( (status) && *status == 2 ) 
        {
                update_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(update_flistp, PIN_FLD_POID, sch_pdp, ebufp );
                when_t  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_WHEN_T, 1, ebufp);

                if( *when_t > 0 )
                {
                        PIN_FLIST_FLD_SET(update_flistp, PIN_FLD_WHEN_T, when_t , ebufp );
                }
                else
                {
                        strcpy(mesg,"Invalid Modification Date");
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, sch_pdp , ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &sched_job_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "55002", ebufp);
                        //PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account schedule job failed", ebufp);
                        strcat(mesg, " - failed");
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, mesg, ebufp);
                        PIN_ERR_LOG_EBUF(PIN_FLD_ERROR_DESCR, mesg, ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        goto CLEANUP;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_update_schedule input flist", update_flistp);
                strcpy(mesg,"Schedule Modification");
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_flistp, &r_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&update_flistp,NULL);

                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Create scheduler error.", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        goto CLEANUP;
                }


        }
        else if( (status) && *status == 3 ) 
        {

		update_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(update_flistp, PIN_FLD_POID, sch_pdp, ebufp );
                new_state  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NEW_STATE,1, ebufp);
                
                if( *new_state == 0 || *new_state == 1 || *new_state == 3 ||  *new_state == 5 ) 
                {
                        PIN_FLIST_FLD_SET(update_flistp, PIN_FLD_STATUS, new_state , ebufp );
                }
		else
                {
                        strcpy(mesg,"Invalid Modification State");
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, sch_pdp , ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &sched_job_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "55002", ebufp);
                        //PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account schedule job failed", ebufp);
                        strcat(mesg, " - failed");
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, mesg, ebufp);
                        PIN_ERR_LOG_EBUF(PIN_FLD_ERROR_DESCR, mesg, ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        goto CLEANUP;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_update_schedule input flist", update_flistp);
                strcpy(mesg,"Schedule Modification");
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_flistp, &r_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&update_flistp,NULL);

                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Create scheduler error.", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        goto CLEANUP;
                }



        }
        else if( (status) && *status == 0 ) 
        {

	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_update_schedule input flist", i_flistp);	
                update_flistp = PIN_FLIST_CREATE(ebufp);
	        PIN_FLIST_FLD_SET(update_flistp, PIN_FLD_POID, sch_pdp, ebufp );
        	when_t  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_WHEN_T, 1, ebufp);
        	new_state  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NEW_STATE, 1, ebufp);
 
                if( *new_state == 0 || *new_state == 1 || *new_state == 3 ||  *new_state == 5 ) 
                {                 
	        	PIN_FLIST_FLD_SET(update_flistp, PIN_FLD_STATUS, new_state , ebufp );
                } 
                if( *when_t > 0 )
                {
	        	PIN_FLIST_FLD_SET(update_flistp, PIN_FLD_WHEN_T, when_t , ebufp );
                }
                else 
                {
                	strcpy(mesg,"Invalid Deletion State");
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, sch_pdp , ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &sched_job_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "55002", ebufp);
			//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account schedule job failed", ebufp);
			strcat(mesg, " - failed");
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, mesg, ebufp);
			PIN_ERR_LOG_EBUF(PIN_FLD_ERROR_DESCR, mesg, ebufp);
			PIN_ERRBUF_RESET(ebufp);
			goto CLEANUP;
                }
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_update_schedule input flist", update_flistp);	
                strcpy(mesg,"Schedule Modification");
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_flistp, &r_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&update_flistp,NULL);

                if (PIN_ERRBUF_IS_ERR(ebufp))
	        {
       		        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Create scheduler error.", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                	goto CLEANUP;
        	}


        } 
        else if( (status) && *status == 1 ) 
        {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_delete input flist", i_flistp);	
                update_flistp = PIN_FLIST_CREATE(ebufp);
	        sch_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	        PIN_FLIST_FLD_SET(update_flistp, PIN_FLD_POID, sch_pdp, ebufp );
        	new_state  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NEW_STATE, 0, ebufp);
                if( *new_state == 5 ) 
                {                 
	        	PIN_FLIST_FLD_SET(update_flistp, PIN_FLD_STATUS, new_state , ebufp );
                } 
                else 
                {
                	strcpy(mesg,"Invalid Deletion State");
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, sch_pdp , ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &sched_job_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "55002", ebufp);
			//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account schedule job failed", ebufp);
			strcat(mesg, " - failed");
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, mesg, ebufp);
			PIN_ERR_LOG_EBUF(PIN_FLD_ERROR_DESCR, mesg, ebufp);
			PIN_ERRBUF_RESET(ebufp);
			goto CLEANUP;		
                } 
                strcpy(mesg,"Schedule Deletion");
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_update_schedule input flist", update_flistp);	
                PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_flistp, &r_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&update_flistp,NULL);

                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Delete scheduler error.", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        goto CLEANUP;
                }

        }
	
	if ( acct_update == 0)
	{	
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_schedule successful");
	        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, sch_pdp, ebufp );
        	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &sched_job_success, ebufp);

	}
	else if ( acct_update == 1)
	{	
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, sch_pdp , ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &sched_job_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "55002", ebufp);
		//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account schedule job failed", ebufp);
		strcat(mesg, " - failed");
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, mesg, ebufp);
		PIN_ERR_LOG_EBUF(PIN_FLD_ERROR_DESCR, mesg, ebufp);
		PIN_ERRBUF_RESET(ebufp);
		goto CLEANUP;		
	}
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, sch_pdp, ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &sched_job_success, ebufp);
	if ( acct_update == 0)
	{	
		strcat(mesg, " - completed successfully");
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, mesg, ebufp);
		
		lc_in_flist = PIN_FLIST_CREATE(ebufp);
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY,0,0, ebufp );
		PIN_FLIST_FLD_COPY(vp, PIN_FLD_POID, lc_in_flist, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, lc_in_flist, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACTION_NAME, lc_in_flist, PIN_FLD_ACTION_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, lc_in_flist, PIN_FLD_SCHEDULE_OBJ, ebufp); 
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_WHEN_T, lc_in_flist, PIN_FLD_WHEN_T, ebufp);
		PIN_FLIST_FLD_SET(lc_in_flist, PIN_FLD_OPCODE, &caller, ebufp);

		fm_mso_create_lifecycle_evt(ctxp, lc_in_flist, NULL, ID_MODIFY_SCHEDULE_ACTION, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_FLIST_DESTROY_EX(&lc_in_flist,NULL);
	}
	else if ( acct_update == 1)
	{
		strcat(mesg, " - failed");
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, mesg, ebufp);
	}
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	CLEANUP:	
	*r_flistpp = PIN_FLIST_COPY(ret_flistp,ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp,NULL);
	return;
}


/**************************************************
* Function: fm_mso_modify_scheduler()
* 
* 
***************************************************/
static int 
fm_mso_modify_scheduler(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	int			action_code,
	int			 opcode,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*planlists_flistp = NULL;
        pin_flist_t		*servlists_flistp = NULL;
	pin_flist_t		*billlists_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	int			 elem_base = 0;
	int			 elem_data = 0;
	char            	*char_progname = NULL;
	char            	*char_buf = NULL;
	pin_cookie_t    	 cookie_base = NULL;
	pin_cookie_t    	 cookie_data = NULL;
	pin_flist_t     	*s_flistp = NULL;
    	pin_flist_t     	*data_flistp = NULL;
    	pin_flist_t     	*r_flistp = NULL;
    	int32            	 len = 0;
    	pin_buf_t       	*pin_bufp = NULL;
        char                    *progname = NULL;
        char                    mod_progname[255] = "pin_deferred_act|";
    	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 1;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_scheduler :");	
	s_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, s_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_WHEN_T, s_flistp, PIN_FLD_WHEN_T, ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, s_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
        progname =  PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 0, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_OPCODE, (void *)&opcode, ebufp);
    
        	
	data_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_DATA_ARRAY,0,0, ebufp );
        strcat(mod_progname,progname);
        PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_PROGRAM_NAME, mod_progname, ebufp);
	//PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_PROGRAM_NAME, "pin_deferred_act", ebufp);
	           		
	if (action_code == MSO_ACTION_ACTIVATE_CUSTOMER) 
	{
        	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "ACTIVATE_CUSTOMER", ebufp);
        	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "ACTIVATE_CUSTOMER", ebufp);

	}

	if (action_code == MSO_ACTION_ADD_PLAN)
	{
        	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "ADD_PLAN", ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "ADD_PLAN  from schedule", ebufp);
	}
	if (action_code == MSO_ACTION_CANCEL_PLAN)
        {
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "CANCEL_PLAN", ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "CANCEL_PLAN from schedule", ebufp);

        }
	if (action_code == MSO_ACTION_CHANGE_PLAN)
        {
        	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "CHANGE_PLAN", ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "CHANGE_PLAN from schedule", ebufp);

        }
        if (action_code == MSO_ACTION_SUSPEND_SERVICE)
        {
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "SUSPEND_SERVICE", ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "SUSPEND_SERVICE from schedule", ebufp);

        }
        if (action_code == MSO_ACTION_REACTIVATE_SERVICE)
        {
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "REACTIVATE_SERVICE", ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "REACTIVATE_SERVICE from schedule", ebufp);

        }

	/* if (action_code == MSO_ACTION_BMAIL)
        {
               PIN_FLIST_ELEM_SET(o_flistp, planlists_flistp, PIN_FLD_PLAN_LISTS, elem_base, ebufp);
         }
	if (action_code == MSO_ACTION_BMAIL)
        {
               PIN_FLIST_ELEM_SET(o_flistp, planlists_flistp, PIN_FLD_PLAN_LISTS, elem_base, ebufp);
        }*/
        if (action_code == MSO_ACTION_TOPUP)
        {
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "TOPUP", ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "TOPUP from schedule", ebufp);
        }
        if (action_code == MSO_ACTION_RENEW)
        {
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "RENEW", ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "RENEW from schedule", ebufp);
        }
        if (action_code == MSO_ACTION_CREDIT_NOTE)
        {
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "CREDIT_NOTE", ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "CREDIT_NOTE from schedule", ebufp);
        }


	/* start of new action*/
        if (action_code == MSO_ACTION_HOLD)
        {
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "HOLD", ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "HOLD from schedule", ebufp);
        }
        if (action_code == MSO_ACTION_UNHOLD)
        {
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACTION_NAME, "UNHOLD", ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SYS_DESCR, "UNHOLD from schedule", ebufp);
        }
	/* End of new action */


     	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in constructing input flist.", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                return 1;
        }
	
	/* Put flist built for MSO_OP_CUST_ADD_PLAN
      	* into buffer of PIN_FLD_INPUT_FLIST
      	*/
     	PIN_FLIST_TO_STR(data_flistp, &char_buf, &len, ebufp);
     	pin_bufp = (pin_buf_t *)calloc(1, sizeof(pin_buf_t));
     	pin_bufp->data = (caddr_t)char_buf;
     	pin_bufp->size = len + 1;
     	pin_bufp->flag = 0;
     	pin_bufp->offset = 0;
     	pin_bufp->xbuf_file = NULL;
     	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_INPUT_FLIST, (void *)pin_bufp, ebufp);
     
     	/* 
      	* Create the schedule object
      	*/
     	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Schedule create input flist", s_flistp);
     	PCM_OP(ctxp, PCM_OP_ACT_SCHEDULE_CREATE, 0, s_flistp, &r_flistp, ebufp);
     	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Schedule create return flist", r_flistp);
    	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Opcode call failed.", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                return 1;
        }
	PIN_FLIST_FLD_COPY(s_flistp, PIN_FLD_ACTION_NAME, in_flist, PIN_FLD_ACTION_NAME, ebufp);
	PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_OBJECT, in_flist, PIN_FLD_SCHEDULE_OBJ, ebufp);

     	if(pin_bufp)
     		 pin_free(pin_bufp);
     	if (char_buf)
     		pin_free(char_buf);
     	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
     	//PIN_FLIST_DESTROY_EX(&data_flistp, NULL);    
     	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);  
     
     	if (PIN_ERRBUF_IS_ERR(ebufp)) 
     	{
     		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Create scheduler error.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
     		return 1;
     	}
	return 0;
}
