#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: pin_mta_test.c:CUPmod7.3PatchInt:1:2007-Feb-07 06:51:33 %";
#endif

#include <stdio.h>
#include "pcm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_mta.h"
#include "pin_flds.h"
#include "mso_ops_flds.h"
#include "mso_commission.h"

time_t          date_passed = 0;

/*******************************************************************
 * Configuration of application
 * Called prior MTA_CONFIG policy opcode
 *******************************************************************/
void
pin_mta_config(
        pin_flist_t        *param_flistp,
        pin_flist_t        *app_flistp,
        pin_errbuf_t    *ebufp)
{
    int32           rec_id = 0;
    pin_cookie_t    cookie = 0;
    pin_cookie_t    prev_cookie = 0;
    pin_flist_t     *flistp = 0;
    char            *option = 0;
    //char	    *business_type =NULL;
    char            *obj_type   = NULL;
    int32           mta_flags = 0;
    int32           err = 0;
    int32           i = 0;
    void            *vp = 0;
    int32          i_param_count = 0;
    int32          bus_type = 0;
    time_t		*date_passed1 = NULL;
    char msg[20];
    char	*dp = NULL;

    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config parameters flist", param_flistp);
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config application info flist", app_flistp);
    
    vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FLAGS, 0,ebufp);
    if(vp)
	mta_flags = *((int32*)vp);

    /***********************************************************
    * The new flag MTA_FLAG_VERSION_NEW has been introduced to
    * differentiate between new mta & old mta applications as
    * only new applications have the ability to drop PIN_FLD_PARAMS
    * for valid parameters . It is only for new apps that
    * checking err_params_cnt is valid, otherwise for old applications
    * this check without a distincion for new applications would hamper
    * normal functioning.
    ***********************************************************/
    mta_flags = mta_flags | MTA_FLAG_VERSION_NEW ;
    i_param_count = PIN_FLIST_ELEM_COUNT(param_flistp, PIN_FLD_PARAMS, ebufp);

    if ( i_param_count == 1)
    {
        while((flistp = PIN_FLIST_ELEM_GET_NEXT(param_flistp, PIN_FLD_PARAMS, &rec_id, 1, &cookie, ebufp))!= NULL) 
        {
            option = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_NAME, 0, ebufp);
            if(option && (strcmp(option, "-end_date") == 0)) 
            {    
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "flistp:", flistp);
                dp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
                if(!dp) {
                    mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                }
                else
                {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Date Passed STR is:");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, dp);
			date_passed = (time_t) atoi(dp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Date Passed INT is:");
			sprintf(msg,"%d",date_passed);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			if ( date_passed == 0) {
				date_passed = pin_virtual_time((time_t *)NULL);
			}
			PIN_FLIST_FLD_SET (app_flistp,  PIN_FLD_END_T, &date_passed, ebufp);
		}

			PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
			cookie = prev_cookie;
            }
            else 
            {
                prev_cookie = cookie;
            }
        }
    }
    else
    {
        mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Invalid input Parameters Passed ");
    }

    date_passed1 = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_END_T, 1, ebufp);
    if(!date_passed1) {
        mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
    }

    PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config application info flist", app_flistp);

    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_config error", ebufp);
    }
    return;
}

/*******************************************************************
 * Usage information for the specific app.
 * Called prior MTA_USAGE policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_usage(
        char    *prog)
{

    pin_errbuf_t        ebuf;
    pin_flist_t        *ext_flistp = NULL;
    char                *usage_str = NULL;
    char                *format = "\nUsage:     %s [-verbose] -end_date <UNIX_DATE> \n\n";
            
    PIN_ERRBUF_CLEAR (&ebuf);
    
    usage_str = (char*)pin_malloc( strlen(format) + strlen(prog) + 1 );

    if (usage_str == NULL) {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No Memory error");
        return;
    }

    sprintf(usage_str, format ,prog);
    ext_flistp = pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO, &ebuf);
    PIN_FLIST_FLD_SET (ext_flistp, PIN_FLD_DESCR, usage_str, &ebuf)

    if (PIN_ERR_IS_ERR(&ebuf)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "pin_mta_usage error", &ebuf);
    }
    if (usage_str) {
        pin_free(usage_str);
    }
    return;
}

/*******************************************************************
 * Usage information for the specific app.
 * Called after MTA_USAGE policy opcode
 * Information passed from customization layer
 * can be processed and displayed here
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_usage(
        pin_flist_t         *param_flistp,
        pin_flist_t         *app_flistp,
        pin_errbuf_t        *ebufp)
{
    pin_flist_t             *ext_flistp = NULL;
    void                    *vp = 0;

    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage parameters flist", param_flistp);
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage application info flist", app_flistp);

    ext_flistp = pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO, ebufp);
    vp = PIN_FLIST_FLD_GET (ext_flistp, PIN_FLD_DESCR, 1, ebufp);

    if(vp) {
        printf("%s",(char*) vp);
    }

    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_post_usage error", ebufp);
    }
    return;
}

/*******************************************************************
 * Application defined search criteria.
 * Called prior MTA_INIT_SEARCH policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_init_search(
        pin_flist_t         *app_flistp,
        pin_flist_t         **s_flistpp,
        pin_errbuf_t        *ebufp)
{

    pin_flist_t         *s_flistp    = NULL;
    pin_flist_t        *tmp_flistp = NULL;
    pin_flist_t        *tmp_flistp1 = NULL;
    char                *template = "select X from /mso_dsa_rule_associate where F1 < V1 or ( F2 >= V2 and F3 < V3 ) ";
    char                obj_type[100];
    int32               s_flags = 0;
    poid_t              *s_pdp = NULL;
    poid_t              *i_pdp = NULL;
    int64               id = -1;
    int64               db = 0;
    int32               rule_id = 0;
    void                *vp = NULL;
    int32               comm_model = 5;


    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    *s_flistpp = 0;

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", app_flistp);
    //date_passed = PIN_FLIST_FLD_GET(app_flistp,PIN_FLD_END_T,0,ebufp);
    
    s_flistp = PIN_FLIST_CREATE(ebufp);

    /***********************************************************
     * Simple search flist
     ***********************************************************
     * 0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
     * 0 PIN_FLD_FLAGS           INT [0] 0
     * 0 PIN_FLD_TEMPLATE        STR [0] "select X from /mso_dsa_rule_associate where F1 < V1 or ( F2 >= V2 and F3 < V3 ) "
     * 0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
     * 1     PIN_FLD_BILL_CYCLE_T    TSTAMP [0] (1425644727)
     * 0 PIN_FLD_ARGS          ARRAY [2] allocated 20, used 1
     * 1     PIN_FLD_END_T    TSTAMP [0] (1425644727)
     * 0 PIN_FLD_ARGS          ARRAY [3] allocated 20, used 1
     * 1     PIN_FLD_START_T    TSTAMP [0] (1425644727)
     * 0 PIN_FLD_RESULTS          ARRAY [0] allocated 20, used 1
     * 1     PIN_FLD_POID    POID [0] NULL
     * 1     MSO_FLD_SALESMAN_OBJ    POID [0] NULL
     * 1     PIN_FLD_BILL_CYCLE_T    TSTAMP [0] (0)
     * 1     PIN_FLD_START_T    TSTAMP [0] (0)
     * 1     MSO_FLD_RULE_ID   INT [0] 0
     * 1     PIN_FLD_END_T    TSTAMP [0] (0)
	***********************************************************/

    vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
    if(vp)
        db = PIN_POID_GET_DB ((poid_t*)vp);

    s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
    PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
    PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
    PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_TEMPLATE,template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_BILL_CYCLE_T, &date_passed, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_END_T, &date_passed, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_START_T, &date_passed, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_RESULTS,0, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_SALESMAN_OBJ, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_BILL_CYCLE_T, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_RULE_ID, &rule_id, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_START_T, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_END_T, NULL, ebufp);

    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
        PIN_FLIST_DESTROY_EX (&s_flistp,0);
    }
    else
    {
        *s_flistpp = s_flistp;
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", s_flistp);
    }

    return;
}
/***************************************************/

/*******************************************************************
 * Function called when new job is avaialble to worker
 * Called prior MTA_WORKER_JOB policy opcode
 *******************************************************************/

PIN_EXPORT void 
pin_mta_post_tune(
	pin_flist_t  *app_flistp,
 	pin_flist_t  *srch_res_flistp,
 	pin_errbuf_t *ebufp)
{
 	pin_flist_t  *rest_flistp=NULL;
 	pin_flist_t  *multirest_flistp=NULL;
 	pin_cookie_t cookie = NULL;
 	int32 	     elem_id = 0;
 	
 	if (PIN_ERR_IS_ERR(ebufp)) {
  		return;
 	}
 	PIN_ERRBUF_CLEAR (ebufp);

 	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune application info flist", app_flistp);
 	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results flist", srch_res_flistp);
 	multirest_flistp = PIN_FLIST_ELEM_GET(srch_res_flistp,PIN_FLD_MULTI_RESULTS,PIN_ELEMID_ANY,1,ebufp);

 	while((rest_flistp = PIN_FLIST_ELEM_GET_NEXT(multirest_flistp, PIN_FLD_RESULTS,
  		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {
  		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results modify flist", rest_flistp);
	} 


 	if (PIN_ERR_IS_ERR(ebufp)) {
  		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_post_tune error", ebufp);
 	}
 	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results output flist", srch_res_flistp);

 	return;
}

/*******************************************************************
 * Main application opcode is called here
 *******************************************************************/

PIN_EXPORT void 
pin_mta_worker_opcode(
        pcm_context_t       *ctxp,
        pin_flist_t        *srch_res_flistp,
        pin_flist_t        *op_in_flistp,
        pin_flist_t        **op_out_flistpp,
        pin_flist_t        *ti_flistp,
        pin_errbuf_t        *ebufp)
{
    pin_flist_t             *w_o_flist = NULL;
    pin_flist_t             *w_inflist = NULL;
	pin_flist_t             *wf_iflistp = NULL;
	pin_flist_t             *wf_oflistp = NULL;
    int32                   *opcode = NULL;
    int32                   *opcode_status = NULL;
    char                    *msg[128];
    char		    *error_descr = NULL;
    char		    *error_code	= NULL;
    char		    err_code[20];
    pin_err_location_t      err_location;
    pin_err_class_t         err_class;
    pin_err_t               pin_err;
    pin_fld_num_t           err_field;
    pin_rec_id_t            err_rec_id;
    pin_err_reserved_t      err_reserved;

    pin_flist_t		    *flistp = NULL;
    pin_flist_t		    *args_flistp = NULL;
    pin_flist_t		    *tmp_flistp = NULL;
    pin_flist_t		    *comm_in_flistp = NULL;
    pin_flist_t		    *lco_serv_flistp = NULL;
    pin_flist_t		    *r_sflistp = NULL;
	pin_flist_t         *in_flistp = NULL;
    poid_t		    *event_poidp = NULL;
    poid_t		    *s_poidp = NULL;
    time_t		    *billed_t = NULL;
    time_t		    billed_new_t = 0;
    int64		    db = 1;
    int64		    status = 10100;
    int32		    src_flag = 256;
	int32		    *ret_status = 0;

    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search results flist", srch_res_flistp);
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode", op_in_flistp);
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode thread info flist", ti_flistp);

	billed_t = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_BILL_CYCLE_T, 0, ebufp);
	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(op_in_flistp, MSO_FLD_SALESMAN_OBJ,in_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_BILL_CYCLE_T,in_flistp, PIN_FLD_BILL_CYCLE_T, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_START_T,in_flistp, PIN_FLD_START_T, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_END_T,in_flistp, PIN_FLD_END_T, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, MSO_FLD_RULE_ID,in_flistp, MSO_FLD_RULE_ID, ebufp );
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_REQ_END_T, &date_passed, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_COMMISSION_PROCESS_DSA_COMMISSION input flist", op_in_flistp);
    /*Prepar the input flist to call the commission opcode end*/
    
    PCM_OP (ctxp, MSO_OP_COMMISSION_PROCESS_DSA_COMMISSION, 0, in_flistp, op_out_flistpp, ebufp);

    

    if(!PIN_ERR_IS_ERR(ebufp))
    {
    		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_COMMISSION_PROCESS_DSA_COMMISSION output flist", *op_out_flistpp);
		ret_status = PIN_FLIST_FLD_GET(*op_out_flistpp, PIN_FLD_STATUS, 1, ebufp);
		if(*ret_status == 0 && (date_passed > *billed_t || *billed_t == billed_new_t))
		{
			wf_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_POID, wf_iflistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(wf_iflistp, PIN_FLD_BILL_CYCLE_T, &date_passed, ebufp);
	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode WRITE_FLDS input flist", wf_iflistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wf_iflistp, &wf_oflistp, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode WRITE_FLDS output flist", wf_oflistp);
	
			if (PIN_ERR_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"pin_mta_worker_opcode PCM_OP_WRITE_FLDS error", ebufp);
			}
			PIN_FLIST_DESTROY_EX(&wf_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&wf_oflistp, NULL);
		}
     }
     PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode DSA commission output flist", *op_out_flistpp);

     PIN_FLIST_DESTROY_EX(&in_flistp, NULL);

cleanup:
    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_worker_opcode error", ebufp);
    }
    else{
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp);
    }
    return;
}
