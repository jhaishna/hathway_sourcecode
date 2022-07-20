
#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: mso_clean_rsvns.c.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/tcf.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_errs.h"
#include "mso_rate.h"

void mso_clean_rsvns_op(
		pcm_context_t	*ctxp,
		pin_flist_t	*search_flistp,
		pin_flist_t	*i_flistp,
		pin_flist_t	**r_flistpp,
		pin_errbuf_t	*ebufp);

static void
mso_worker_rsvns_get_daily_usage(
        pcm_context_t           *ctxp,
        pin_flist_t             *session_flistp,
	pin_flist_t		**ret_flistpp,
        pin_errbuf_t            *ebufp );

static void
mso_worker_rsvns_update_daily_usage(
        pcm_context_t           *ctxp,
        pin_flist_t             *session_flistp,
        pin_flist_t             *daily_usg_flistp,
	pin_flist_t		*reservation_flistp,
        pin_errbuf_t            *ebufp );

/*******************************************************************
* Main application opcode is called here
*******************************************************************/
PIN_EXPORT void 
mso_mta_worker_opcode(
                              pcm_context_t  *ctxp,
                              pin_flist_t            *srch_res_flistp,
                              pin_flist_t            *op_in_flistp,
                              pin_flist_t            **op_out_flistpp,
                              pin_flist_t            *ti_flistp,
                              pin_errbuf_t       *ebufp)
{
               if (PIN_ERR_IS_ERR(ebufp)) {
                              return;
               }
               PIN_ERRBUF_CLEAR (ebufp);

               PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
                              "pin_mta_worker_opcode search results flist", 
                                                srch_res_flistp);

               PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
                              "pin_mta_worker_opcode prepared flist for main opcode", 
                                                op_in_flistp);

               PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
                                "pin_mta_worker_opcode thread info flist", 
                                                ti_flistp);


               mso_clean_rsvns_op (ctxp, srch_res_flistp, op_in_flistp, op_out_flistpp, ebufp);

               if (PIN_ERR_IS_ERR(ebufp)) {
                       PIN_ERR_LOG_EBUF (PIN_ERR_LEVEL_DEBUG, 
                               "pin_mta_worker_opcode main opcode error", ebufp);
               }
                              
               return;
}


/*******************************************************************
* pin_clean_rsvns_op():
*             cleanup all expired reservation objects & asos      
*******************************************************************/
void
mso_clean_rsvns_op(
		pcm_context_t	*ctxp,
		pin_flist_t	*search_flistp,
		pin_flist_t	*i_flistp,
		pin_flist_t	**r_flistpp,
		pin_errbuf_t	*ebufp)
{
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*session_flistp = NULL;
	pin_flist_t		*cancel_flistp = NULL;
	pin_flist_t		*r_cancel_flistp = NULL;
	pin_flist_t		*stop_flistp = NULL;
	pin_flist_t		*r_stop_flistp = NULL;
	pin_flist_t		*r_del_flistp = NULL;
	pin_flist_t		*extn_info_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*aso_flistp = NULL;
	pin_flist_t		*reserve_flistp = NULL;
	pin_flist_t		*telco_flistp = NULL;
	pin_flist_t		*extd_sesn_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	pin_flist_t		*app_flistp = NULL;
	pin_flist_t		*daily_usg_flistp = NULL;
	poid_t			*poid_p = NULL;
        poid_t			*acc_obj_p = NULL;
	void			*vp = NULL;
	char			*prog_name = "mso_clean_rsvns";
	char			*origin_nw = NULL;
	char			*auth_id = NULL;
	pin_cookie_t		cookie = NULL;
	int32			term_cause = 0;
	int32			elemid = 0;
	int32			status = 0;
	poid_t			*srvc_poid = NULL;
	char			*obj_type =NULL ;
	poid_t			*session_poid = NULL;
	int32			account = PIN_BOOLEAN_TRUE;
	int32			deleted_flag = 3;
	time_t			*end_t = NULL;
	time_t			*expr_t = NULL;
	time_t			curr_t = pin_virtual_time((time_t *)NULL);
	char			errbuf[256] = {""};

	static pin_decimal_t	*zero_qty = NULL;


	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,"mso_worker_rsvns : search flist", search_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,"mso_worker_rsvns : input flist", i_flistp);

        /* Get the optional parameters from the input flist */
	app_flistp = (pin_flist_t *)pin_mta_global_flist_node_get_with_lock( PIN_FLD_APPLICATION_INFO, ebufp );
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,"mso_worker_rsvns : app flist", app_flistp );
	
	vp = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_ACC_FLAG, 1, ebufp);
	if(vp){
		account = *((int32 *)vp);
	}

	vp = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_TERMINATE_CAUSE, 1, ebufp);
        if(vp){
                term_cause = *((int32 *)vp);
        }
	pin_mta_global_flist_node_release (PIN_FLD_APPLICATION_INFO, ebufp );

        /*******************************************************************
         * For each reservation object returned
        *******************************************************************/

        poid_p = (poid_t *)PIN_FLIST_FLD_GET(search_flistp, PIN_FLD_POID, 0, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_worker_rsvns: Unable to get the POID", ebufp);
		goto cleanup;
	}

	/* Get the Associated Session POID */ 
	flistp = PIN_FLIST_CREATE(ebufp); 
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, poid_p, ebufp); 
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "mso_worker_rsvns: Reservation object read input flist", flistp );
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, flistp, &res_flistp, ebufp); 
	if (PIN_ERR_IS_ERR(ebufp)) { 
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_worker_rsvns: reservation object Not Found", ebufp); 
		goto cleanup; 
	}
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "mso_worker_rsvns: Reservation object read output flist", res_flistp );
               
	session_poid = (poid_t*) PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_SESSION_OBJ, 0, ebufp); 
	expr_t = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_EXPIRATION_T, 1, ebufp);

	aso_flistp = PIN_FLIST_CREATE(ebufp); 
	PIN_FLIST_FLD_SET(aso_flistp, PIN_FLD_POID, session_poid, ebufp);
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "mso_worker_rsvns: session input flist", aso_flistp );
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, aso_flistp, &session_flistp, ebufp); 
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,"mso_worker_rsvns : Active Session Object Fields", session_flistp);
	if (PIN_ERR_IS_ERR(ebufp)) { 
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_worker_rsvns: Session Object Not Found", ebufp); 
		goto cleanup; 
	}

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,"mso_worker_rsvns : Active Session Object Fields", session_flistp);

	/* Find out OBJECT type */
	obj_type = "broadband";

	vp = PIN_FLIST_FLD_GET(session_flistp, PIN_FLD_STATUS, 0, ebufp); 
	if(vp) { 
		status = *((int32*)vp); 
	} 

	srvc_poid =  (poid_t*) PIN_FLIST_FLD_GET(session_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp); 
	auth_id =  (char*) PIN_FLIST_FLD_GET(session_flistp, PIN_FLD_ACTIVE_SESSION_ID, 0, ebufp); 
	//prog_name =  (char*) PIN_FLIST_FLD_GET(session_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp); 
	end_t =  (time_t*) PIN_FLIST_FLD_GET(session_flistp, PIN_FLD_END_T, 1, ebufp);

	telco_flistp = PIN_FLIST_SUBSTR_GET(session_flistp, PIN_FLD_TELCO_INFO, 1, ebufp); 


	stop_flistp = PIN_FLIST_CREATE(ebufp); 


	extn_info_flistp = PIN_FLIST_CREATE(ebufp); 
	PIN_FLIST_FLD_COPY(session_flistp,PIN_FLD_SESSION_INFO,extn_info_flistp, PIN_FLD_SESSION_INFO, ebufp);
	extd_sesn_flistp = PIN_FLIST_SUBSTR_GET(extn_info_flistp, PIN_FLD_SESSION_INFO, 1, ebufp); 
	PIN_FLIST_FLD_COPY(extd_sesn_flistp,PIN_FLD_BYTES_DOWNLINK,stop_flistp, PIN_FLD_BYTES_DOWNLINK, ebufp);
	PIN_FLIST_FLD_COPY(extd_sesn_flistp,PIN_FLD_BYTES_UPLINK,stop_flistp, PIN_FLD_BYTES_UPLINK, ebufp);

	PIN_FLIST_FLD_COPY(telco_flistp,PIN_FLD_USER_NAME,extd_sesn_flistp, PIN_FLD_USER_NAME, ebufp);
	PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_FRAMED_IP_ADDRESS,extd_sesn_flistp, MSO_FLD_FRAMED_IP_ADDRESS, ebufp);
	PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_NE_IP_ADDRESS,extd_sesn_flistp, MSO_FLD_NE_IP_ADDRESS, ebufp);
	PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_NE_ID,extd_sesn_flistp, MSO_FLD_NE_ID, ebufp);
	PIN_FLIST_FLD_COPY(telco_flistp,PIN_FLD_SERVICE_CODE,extd_sesn_flistp, PIN_FLD_SERVICE_CODE, ebufp);
	PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_REMOTE_ID,extd_sesn_flistp, MSO_FLD_REMOTE_ID, ebufp);


	bb_info_flistp = PIN_FLIST_CREATE(ebufp); 
	PIN_FLIST_FLD_COPY(telco_flistp,PIN_FLD_USER_NAME,bb_info_flistp, PIN_FLD_USER_NAME, ebufp);
	PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_FRAMED_IP_ADDRESS,bb_info_flistp, MSO_FLD_FRAMED_IP_ADDRESS, ebufp);
	PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_NE_IP_ADDRESS,bb_info_flistp, MSO_FLD_NE_IP_ADDRESS, ebufp);
	PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_NE_ID,bb_info_flistp, MSO_FLD_NE_ID, ebufp);
	PIN_FLIST_FLD_COPY(telco_flistp,PIN_FLD_SERVICE_CODE,bb_info_flistp, PIN_FLD_SERVICE_CODE, ebufp);
	PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_REMOTE_ID,bb_info_flistp, MSO_FLD_REMOTE_ID, ebufp);
	PIN_FLIST_FLD_COPY(telco_flistp,MSO_FLD_OLD_NETWORKID,bb_info_flistp, MSO_FLD_OLD_NETWORKID, ebufp);

	PIN_FLIST_SUBSTR_SET(stop_flistp, (void*)bb_info_flistp, MSO_FLD_BB_INFO, ebufp);


	/* STOP_ACCOUNTING  PIN_FLD_END_TIME: 
	*  1. Use PIN_FLD_END_T in /active_session if available 
	*  2. Else, use PIN_FLD_EXPRIATION_T in /reservation 
	*  3. Else, use the pin_virtual_time current time 
	*/ 
	if (!end_t || (*end_t == 0)) { 
		if (expr_t && (*expr_t != 0)) { 
			end_t = expr_t; 
		} else { 
			end_t = &curr_t; 
		} 
	}

	/* STOP_ACCOUNTING PIN_FLD_BYTES_DOWNLINK: 
	* 1. Use PIN_FLD_BYTES_DOWNLINK in /active_session if available 
	* 2. Else, use PIN_FLD_BYTES_DOWNLINK provided in commandline 
	*/ 
	if (zero_qty == NULL) { 
		zero_qty = pbo_decimal_from_str("0", ebufp); 
	} 

	/*** IF STATUS = CREATED ***/ 
	if( status == 2 ) {

		/***   IF ACC_FLAG = TRUE   ***/ 
		if( account == PIN_BOOLEAN_TRUE ) { 

			/* Prepare input flist and call TCF_AAA_STOP_ACCOUNTING */
			PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_POID, (void*)srvc_poid, ebufp); 
			PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_AUTHORIZATION_ID, (void*)auth_id, ebufp); 
			PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_PROGRAM_NAME, (void*)prog_name, ebufp); 
			PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_ORIGIN_NETWORK, (void*)origin_nw, ebufp);
	
			/* PIN_FLD_OBJ_TYPE is optional to TCF_AAA_STOP_ACCOUNTING */ 
			if (obj_type[0] != '\0') { 
				PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_OBJ_TYPE, (void*)obj_type, ebufp); 
			} 
			PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_END_T, (void*)end_t, ebufp); 
			PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_TERMINATE_CAUSE, (void*)&term_cause, ebufp); 
			PIN_FLIST_SUBSTR_SET(stop_flistp, (void*)extn_info_flistp, PIN_FLD_EXTENDED_INFO, ebufp);

			PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,"mso_worker_rsvns : stop accounting input flist", i_flistp);
			PCM_OP(ctxp, PCM_OP_TCF_AAA_STOP_ACCOUNTING, 0, stop_flistp, &r_stop_flistp, ebufp); 
			if (PIN_ERR_IS_ERR(ebufp)) { 
				sprintf(errbuf, "STOP ACCOUNTING failed for the active session: %s", auth_id); 
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errbuf, ebufp); 
				PIN_ERR_CLEAR_ERR(ebufp); 
			} 
			PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,"mso_worker_rsvns : stop accounting return flist", i_flistp);
		}
	
		else { 
			/* Prepare input flist and call TCF_AAA_CANCEL_AUTHORIZE */
	
			cancel_flistp = PIN_FLIST_CREATE(ebufp); 
			PIN_FLIST_FLD_SET(cancel_flistp, PIN_FLD_POID, (void*)srvc_poid, ebufp); 
			PIN_FLIST_FLD_SET(cancel_flistp, PIN_FLD_AUTHORIZATION_ID, (void*)auth_id, ebufp); 
			PIN_FLIST_FLD_SET(cancel_flistp, PIN_FLD_DELETED_FLAG, (void*)&deleted_flag, ebufp); 
			PIN_FLIST_FLD_SET(cancel_flistp, PIN_FLD_PROGRAM_NAME, (void*)prog_name, ebufp);
                                             
			PCM_OP(ctxp, PCM_OP_TCF_AAA_CANCEL_AUTHORIZATION, 0, cancel_flistp, &r_cancel_flistp, ebufp); 
			if (PIN_ERR_IS_ERR(ebufp)) { 
				sprintf(errbuf, "CANCEL AUTHORIZATION failed for the active session: %s", auth_id); 
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errbuf, ebufp); 
				PIN_ERR_CLEAR_ERR(ebufp); 
			}
			else{
				mso_worker_rsvns_get_daily_usage( ctxp, session_flistp, &daily_usg_flistp, ebufp );
				if( PIN_ERR_IS_ERR(ebufp)){
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "mso_worker_rsvns : daily usage search error", ebufp );
					goto cleanup;
				}

				mso_worker_rsvns_update_daily_usage( ctxp, session_flistp, daily_usg_flistp, res_flistp, ebufp );
				if( PIN_ERR_IS_ERR(ebufp)){
					PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "mso_worker_rsvns : daily usage update error", ebufp );
					goto cleanup;
				}
			}
		} 
	}

	/*** IF STATUS = STARTED/UPDATED ***/

	if( status == 4 || status == 3 ) { 
		/* Prepare input flist and call TCF_AAA_STOP_ACCOUNTING */

		PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_POID, (void*)srvc_poid, ebufp); 
		PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_AUTHORIZATION_ID, (void*)auth_id, ebufp); 
		PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_PROGRAM_NAME, (void*)prog_name, ebufp); 
		PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_ORIGIN_NETWORK, (void*)origin_nw, ebufp);

		/* PIN_FLD_OBJ_TYPE is optional to TCF_AAA_STOP_ACCOUNTING */ 
		if (obj_type[0] != '\0') { 
			PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_OBJ_TYPE, (void*)obj_type, ebufp); 
		} 

		PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_END_T, (void*)end_t, ebufp); 
		PIN_FLIST_FLD_SET(stop_flistp, PIN_FLD_TERMINATE_CAUSE, (void*)&term_cause, ebufp); 
		PIN_FLIST_SUBSTR_SET(stop_flistp, (void*)extn_info_flistp, PIN_FLD_EXTENDED_INFO, ebufp);

		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,"mso_worker_rsvns : stop accounting input flist", i_flistp);
		PCM_OP(ctxp, PCM_OP_TCF_AAA_STOP_ACCOUNTING, 0, stop_flistp, &r_stop_flistp, ebufp); 
		if (PIN_ERR_IS_ERR(ebufp)) { 
			sprintf(errbuf, "STOP ACCOUNTING failed for the active session: %s", auth_id); 
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errbuf, ebufp); 
			PIN_ERR_CLEAR_ERR(ebufp); 
		} 
	}

cleanup: 
		PIN_FLIST_DESTROY_EX(&flistp, NULL); 
		PIN_FLIST_DESTROY_EX(&session_flistp, NULL); 
		PIN_FLIST_DESTROY_EX(&cancel_flistp, NULL); 
		PIN_FLIST_DESTROY_EX(&stop_flistp, NULL); 
		PIN_FLIST_DESTROY_EX(&r_cancel_flistp, NULL); 
		PIN_FLIST_DESTROY_EX(&r_stop_flistp, NULL); 
		PIN_FLIST_DESTROY_EX(&r_del_flistp, NULL); 
		PIN_FLIST_DESTROY_EX(&extn_info_flistp, NULL); 
		PIN_FLIST_DESTROY_EX(&res_flistp, NULL); 
		PIN_FLIST_DESTROY_EX(&aso_flistp, NULL); 
		PIN_FLIST_DESTROY_EX(&reserve_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&daily_usg_flistp, NULL);

	return;
}



static void
mso_worker_rsvns_get_daily_usage(
	pcm_context_t		*ctxp,
	pin_flist_t		*session_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp )
{
	pin_flist_t		*srch_iflistp	= NULL;
	pin_flist_t		*srch_oflistp	= NULL;
	pin_flist_t		*args_flistp	= NULL;

	poid_t			*srch_pdp	= NULL;
	poid_t			*acct_pdp	= NULL;

	char			*srch_tmplt	= NULL;

	int64			db		= 0;
	int			srch_flag	= SRCH_EXACT;

	time_t			start_t		= 0;
	void			*vp		= NULL;
	time_t			day_t		= 0;
	struct tm		ltm;
	struct tm		daytm;

	if( PIN_ERR_IS_ERR(ebufp)){
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "mso_worker_rsvns_update_daily_usage :"
		"Input flist", session_flistp );

	acct_pdp = PIN_FLIST_FLD_GET( session_flistp, PIN_FLD_ACCOUNT_OBJ,
				0, ebufp );
	db = PIN_POID_GET_DB( acct_pdp );

	vp = PIN_FLIST_FLD_GET( session_flistp, PIN_FLD_START_T,
				1, ebufp );
	if( vp ){
		start_t = *(time_t *)vp;
		ltm = *localtime(&start_t);

		daytm.tm_mday = ltm.tm_mday;
		daytm.tm_mon = ltm.tm_mon;
		daytm.tm_year = ltm.tm_year;
		daytm.tm_hour = 0;
		daytm.tm_min = 0;
		daytm.tm_sec = 0;

		day_t = mktime(&daytm);
	}
	

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE( db, "/search", -1, ebufp );
	PIN_FLIST_FLD_PUT( srch_iflistp, PIN_FLD_POID,
		(void *)srch_pdp, ebufp );
	PIN_FLIST_FLD_SET( srch_iflistp, PIN_FLD_FLAGS,
		(void *)&srch_flag, ebufp );

	srch_tmplt = "select X from /mso_bb_daily_usage where "
		"F1 = V1 and F2 = V2 ";
	PIN_FLIST_FLD_SET( srch_iflistp, PIN_FLD_TEMPLATE,
		(void *)srch_tmplt, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD( srch_iflistp,
		PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET( args_flistp, PIN_FLD_ACCOUNT_OBJ,
		(void *)acct_pdp, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD( srch_iflistp,
		PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET( args_flistp, PIN_FLD_EFFECTIVE_T,
		(void *)&day_t, ebufp );

	PIN_FLIST_ELEM_SET( srch_iflistp, NULL, 
		PIN_FLD_RESULTS, 0, ebufp );

	/************************************
	 * Log input flist
	 ************************************/
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
		"mso_worker_rsvns_update_daily_usage :"
		" usage search input flist", srch_iflistp );

	PCM_OP( ctxp, PCM_OP_SEARCH, 0, srch_iflistp,
		&srch_oflistp, ebufp );
	/************************************
	 * Check error
	 ************************************/
	if( PIN_ERR_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"mso_worker_rsvns_update_daily_usage :"
			"Usage search error", ebufp );
		PIN_FLIST_DESTROY_EX( &srch_oflistp, NULL );
	}
	else{
		PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
			"mso_worker_rsvns_update_daily_usage :"
			"Usage search output flist", srch_oflistp );
		*ret_flistpp = srch_oflistp;
	}
	PIN_FLIST_DESTROY_EX( &srch_iflistp, NULL );
	return;
}



static void
mso_worker_rsvns_update_daily_usage(
	pcm_context_t		*ctxp,
	pin_flist_t		*session_flistp,
	pin_flist_t		*daily_usg_flistp,
	pin_flist_t		*reservation_flistp,
	pin_errbuf_t		*ebufp )
{
	pin_flist_t		*upd_iflistp		= NULL;
	pin_flist_t		*upd_oflistp		= NULL;
	pin_flist_t		*results_flistp		= NULL;
	pin_flist_t		*session_info_flistp	= NULL;
	pin_flist_t		*fup_flistp		= NULL;
	pin_flist_t		*cons_flistp		= NULL;

	pin_decimal_t		*fup_amount		= NULL;
	pin_decimal_t		*cons_amount		= NULL;
	pin_decimal_t		*bytes_uplink		= NULL;
	pin_decimal_t		*bytes_downlink		= NULL;
	pin_decimal_t		*sess_bytes_uplink	= NULL;
	pin_decimal_t		*sess_bytes_downlink	= NULL;
	pin_decimal_t		*new_bytes_uplink	= NULL;
	pin_decimal_t		*new_bytes_downlink	= NULL;
	pin_decimal_t		*fup_bytes_uplink	= NULL;
	pin_decimal_t		*fup_bytes_downlink	= NULL;
	pin_decimal_t		*aft_fup_bytes_uplink	= NULL;
	pin_decimal_t		*aft_fup_bytes_downlink	= NULL;
	pin_decimal_t		*total_bytes		= NULL;
	pin_decimal_t		*fup_total_bytes	= NULL;
	pin_decimal_t		*aft_fup_total_bytes	= NULL;
	pin_decimal_t		*new_total_bytes	= pin_decimal( "0", ebufp );
	pin_decimal_t		*zero			= pin_decimal( "0", ebufp );

	char			*str_new_duration	= NULL;
	void			*vp			= NULL;

	int			daily_duration		= 0;
	int			new_duration		= 0;
	int			sess_duration		= 0;
	
	if( PIN_ERR_IS_ERR(ebufp)){
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
		"mso_worker_rsvns_update_daily_usage: "
		"Daily usage flist", daily_usg_flistp );
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
		"mso_worker_rsvns_update_daily_usage: "
		"Session flist", session_flistp );
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
		"mso_worker_rsvns_update_daily_usage: "
		"Reservation flist", reservation_flistp );
	results_flistp = PIN_FLIST_ELEM_GET( daily_usg_flistp,
		PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	if( results_flistp ){
		bytes_uplink = PIN_FLIST_FLD_GET( results_flistp, 
			PIN_FLD_BYTES_UPLINK, 0, ebufp );
		bytes_downlink = PIN_FLIST_FLD_GET( results_flistp,
			PIN_FLD_BYTES_DOWNLINK, 0, ebufp );
		total_bytes = PIN_FLIST_FLD_GET( results_flistp,
			MSO_FLD_TOTAL_DATA, 0, ebufp );

		daily_duration = atoi(PIN_FLIST_FLD_GET( results_flistp, 
			PIN_FLD_DURATION, 0, ebufp ));

		fup_bytes_uplink = PIN_FLIST_FLD_GET( results_flistp,
			MSO_FLD_BYTES_UPLINK_BEF_FUP, 0, ebufp );
		fup_bytes_downlink = PIN_FLIST_FLD_GET( results_flistp,
			MSO_FLD_BYTES_DOWNLINK_BEF_FUP, 0, ebufp );
		fup_total_bytes = PIN_FLIST_FLD_GET( results_flistp,
			MSO_FLD_TOTAL_BEF_FUP, 0, ebufp );

		aft_fup_bytes_uplink = PIN_FLIST_FLD_GET( results_flistp,
			MSO_FLD_BYTES_UPLINK_AFT_FUP, 0, ebufp );
		aft_fup_bytes_downlink = PIN_FLIST_FLD_GET( results_flistp,
			MSO_FLD_BYTES_DOWNLINK_AFT_FUP, 0, ebufp );
		aft_fup_total_bytes = PIN_FLIST_FLD_GET( results_flistp,
			MSO_FLD_TOTAL_AFT_FUP, 0, ebufp );
	}
	session_info_flistp = PIN_FLIST_ELEM_GET( session_flistp,
		PIN_FLD_SESSION_INFO, PIN_ELEMID_ANY, 1, ebufp );
	if( session_info_flistp ){
		sess_bytes_uplink = PIN_FLIST_FLD_GET( session_info_flistp,
			PIN_FLD_BYTES_UPLINK, 0, ebufp );
		sess_bytes_downlink = PIN_FLIST_FLD_GET( session_info_flistp,
			PIN_FLD_BYTES_DOWNLINK, 0, ebufp );
		vp = PIN_FLIST_FLD_GET( session_info_flistp,
			PIN_FLD_DURATION_SECONDS, 0, ebufp );
		if( vp ){
			sess_duration = *(int *)vp;
		}
	}
	if( PIN_ERR_IS_ERR(ebufp) ){
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"mso_worker_rsvns_update_daily_usage :"
			"Fields value getting error", ebufp );
		goto cleanup;
	}
	
	if( results_flistp && session_info_flistp ){

		upd_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY( results_flistp, PIN_FLD_POID,
			upd_iflistp, PIN_FLD_POID, ebufp );

		if( !pbo_decimal_is_null( total_bytes, ebufp )){
			pbo_decimal_add_assign( new_total_bytes,
				total_bytes, ebufp );
		}
		if( !pbo_decimal_is_null(bytes_uplink, ebufp) && 
				!pbo_decimal_is_null(sess_bytes_uplink, ebufp) ){
			new_bytes_uplink = pbo_decimal_subtract( bytes_uplink,
				sess_bytes_uplink, ebufp );
			pbo_decimal_subtract_assign( new_total_bytes, sess_bytes_uplink,
				ebufp );
			PIN_FLIST_FLD_PUT( upd_iflistp, PIN_FLD_BYTES_UPLINK,
				(void *)new_bytes_uplink, ebufp );
		}

		if( !pbo_decimal_is_null(bytes_downlink, ebufp) &&
				!pbo_decimal_is_null(sess_bytes_downlink, ebufp) ){
			new_bytes_downlink = pbo_decimal_subtract( bytes_downlink,
				sess_bytes_downlink, ebufp );
			pbo_decimal_subtract_assign( new_total_bytes, sess_bytes_downlink,
				ebufp );
			PIN_FLIST_FLD_PUT( upd_iflistp, PIN_FLD_BYTES_DOWNLINK,
				(void *)new_bytes_downlink, ebufp );
		}
		new_duration = daily_duration - sess_duration;
		str_new_duration = (char *)malloc( sizeof(new_duration) );
		memset( str_new_duration, '\0', sizeof(new_duration) );
		sprintf( str_new_duration, "%d", new_duration );

		if( new_total_bytes && 
			!pbo_decimal_is_zero( new_total_bytes, ebufp )){

			PIN_FLIST_FLD_SET( upd_iflistp, MSO_FLD_TOTAL_DATA,
				(void *)new_total_bytes, ebufp );
		}
		PIN_FLIST_FLD_PUT( upd_iflistp, PIN_FLD_DURATION,
			(void *)str_new_duration, ebufp );

		if( (fup_bytes_uplink && !pbo_decimal_is_zero( fup_bytes_uplink, ebufp )) || 
			(fup_bytes_downlink && !pbo_decimal_is_zero( fup_bytes_downlink, ebufp))){

			fup_flistp = PIN_FLIST_ELEM_GET( reservation_flistp,
				PIN_FLD_BALANCES, 1000104, 1, ebufp );
			if( fup_flistp ){
				fup_amount = PIN_FLIST_FLD_GET( fup_flistp, 
					PIN_FLD_AMOUNT, 1, ebufp );
			}
			cons_flistp = PIN_FLIST_ELEM_GET( reservation_flistp,
				PIN_FLD_BALANCES, 1000103, 1, ebufp );
			if( fup_flistp ){
				cons_amount = PIN_FLIST_FLD_GET( cons_flistp,
					PIN_FLD_AMOUNT, 1, ebufp );
			}

			/*********************************************
			 * Calculation of Fup data
			 *********************************************/
			if( fup_amount && !pbo_decimal_is_zero( fup_amount, ebufp )){
				pbo_decimal_multiply_assign( new_total_bytes, zero, ebufp );
				if( !pbo_decimal_is_null( fup_total_bytes, ebufp )){
					pbo_decimal_add_assign( new_total_bytes,
						fup_total_bytes, ebufp );
				}
				if( !pbo_decimal_is_null(fup_bytes_uplink, ebufp) &&
						!pbo_decimal_is_null(sess_bytes_uplink, ebufp) ){
					new_bytes_uplink = pbo_decimal_subtract( fup_bytes_uplink,
						sess_bytes_uplink, ebufp );
					pbo_decimal_subtract_assign( new_total_bytes, sess_bytes_uplink,
						ebufp );
					PIN_FLIST_FLD_PUT( upd_iflistp, MSO_FLD_BYTES_UPLINK_BEF_FUP,
						(void *)new_bytes_uplink, ebufp );
				}
				if( !pbo_decimal_is_null(fup_bytes_downlink, ebufp) &&
						!pbo_decimal_is_null(sess_bytes_downlink, ebufp) ){
					new_bytes_downlink = pbo_decimal_subtract( fup_bytes_downlink,
						sess_bytes_downlink, ebufp );
					pbo_decimal_subtract_assign( new_total_bytes, sess_bytes_downlink,
						ebufp );
					PIN_FLIST_FLD_PUT( upd_iflistp, MSO_FLD_BYTES_DOWNLINK_BEF_FUP,
						(void *)new_bytes_downlink, ebufp );
				}
				if( new_total_bytes && !pbo_decimal_is_zero( new_total_bytes, ebufp )){
					PIN_FLIST_FLD_SET( upd_iflistp, MSO_FLD_TOTAL_BEF_FUP,
						(void *)new_total_bytes, ebufp );
				}
			}
			else{
				pbo_decimal_multiply_assign( new_total_bytes, zero, ebufp );
				if( !pbo_decimal_is_null( aft_fup_total_bytes, ebufp )){
					pbo_decimal_add_assign( new_total_bytes,
						aft_fup_total_bytes, ebufp );
				}
				if( !pbo_decimal_is_null(aft_fup_bytes_uplink, ebufp) &&
						!pbo_decimal_is_null(sess_bytes_uplink, ebufp) ){
					new_bytes_uplink = pbo_decimal_subtract( aft_fup_bytes_uplink,
						sess_bytes_uplink, ebufp );
					pbo_decimal_subtract_assign( new_total_bytes, sess_bytes_uplink,
						ebufp );
					PIN_FLIST_FLD_PUT( upd_iflistp, MSO_FLD_BYTES_UPLINK_BEF_FUP,
						(void *)new_bytes_uplink, ebufp );
				}
				if( !pbo_decimal_is_null( aft_fup_bytes_downlink, ebufp) &&
						!pbo_decimal_is_null(sess_bytes_downlink, ebufp) ){
					new_bytes_downlink = pbo_decimal_subtract( aft_fup_bytes_downlink,
						sess_bytes_downlink, ebufp );
					pbo_decimal_subtract_assign( new_total_bytes, sess_bytes_downlink,
						ebufp );
					PIN_FLIST_FLD_PUT( upd_iflistp, MSO_FLD_BYTES_DOWNLINK_BEF_FUP,
						(void *)new_bytes_downlink, ebufp );
				}
				if( new_total_bytes && !pbo_decimal_is_zero( new_total_bytes, ebufp )){
					PIN_FLIST_FLD_SET( upd_iflistp, MSO_FLD_TOTAL_BEF_FUP,
						(void *)new_total_bytes, ebufp );
				}
			}

		}

		/**************************************
		 * Log update input flist
		 **************************************/
		PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
			"mso_worker_rsvns_update_daily_usage :"
			"Update input flist", upd_iflistp );

		PCM_OP( ctxp, PCM_OP_WRITE_FLDS, 0, 
			upd_iflistp, &upd_oflistp, ebufp );

		/*************************************
		 * Check Error
		 *************************************/
		if( PIN_ERR_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
				"mso_worker_rsvns_update_daily_usage :"
				"Update daily usage error", ebufp );
		}
		else{
			PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
				"mso_worker_rsvns_update_daily_usage :"
				"Update daily usage output flist", upd_oflistp );
		}
		PIN_FLIST_DESTROY_EX( &upd_iflistp, NULL );
		PIN_FLIST_DESTROY_EX( &upd_oflistp, NULL );
	}

	cleanup:
	if( new_total_bytes ){
		pbo_decimal_destroy( &new_total_bytes );
	}
	if( zero ){
		pbo_decimal_destroy( &zero );
	}
}

