/*******************************************************************
 *
 *    Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *    This material is the confidential property of Oracle Corporation
 *    or its licensors and may be used, reproduced, stored or transmitted
 *    only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: pin_mta_test.c:CUPmod7.3PatchInt:1:2007-Feb-07 06:51:33 %";
#endif

#include <stdio.h>
#include "pcm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_mta.h"
#include "pin_flds.h"

#define  NEW_RECORD     0  
#define  SUCCESS_RECORD     1
#define  ERROR_RECORD     2



/*******************************************************************
 * During real application development, there is no necessity to implement 
 * empty MTA call back functions. Application will recognize implemented
 * functions automatically during run-time.
 *******************************************************************/

/*******************************************************************
 * Configuration of application
 * Called prior MTA_CONFIG policy opcode
 *******************************************************************/
PIN_EXPORT void 
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
    char            *object_type   = NULL;
    char            *obj_type   = NULL;
    int32           mta_flags = 0;
    int32           err = 0;
    int32           i = 0;
    void            *vp = 0;
    int32          i_param_count = 0;
    char msg[20];
    pid_t   v_ppid = 0;
    pid_t   v_gpid = 0;	

    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config parameters flist", 
                       param_flistp);
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config application info flist", 
                           app_flistp);
    /**added changes to check mta is called directly or by script***/
        v_ppid = getpid();
        v_gpid = getpgid(0);
        if (v_gpid == v_ppid)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                	"MTA called directly", ebufp);
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, 0, 0, 0);
                return;
        }
        else
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called from script");
        }
	 
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
            /***************************************************
             * Test options options.
             ***************************************************/
            if(option && (strcmp(option, "-object_type") == 0)) 
            {    

                object_type = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);

                
                if(!object_type) {
                    mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                }
                else
                {
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, object_type);
                    PIN_FLIST_FLD_COPY (flistp, PIN_FLD_PARAM_VALUE, app_flistp,  PIN_FLD_OBJ_TYPE, ebufp);
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

    object_type = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_OBJ_TYPE, 1, ebufp);
    if(!object_type) {
        mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
    }

    PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config application info flist", 
                       app_flistp);

    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "pin_mta_config error", ebufp);
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
    char                *format = "\nUsage:     %s [-verbose] -object_type </mso_mta_job/xxx> \n\n";
            
    PIN_ERRBUF_CLEAR (&ebuf);
    
    usage_str = (char*)pin_malloc( strlen(format) + strlen(prog) + 1 );

    if (usage_str == NULL) {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No Memory error");
        return;
    }

    sprintf(usage_str, format ,prog);

    ext_flistp = pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO,
                               &ebuf);

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

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage parameters flist", 
                       param_flistp);
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage application info flist", 
                       app_flistp);

    ext_flistp =
        pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO,
                                               ebufp);

    vp = PIN_FLIST_FLD_GET (ext_flistp, PIN_FLD_DESCR, 1, ebufp);

    if(vp) {
        printf("%s",(char*) vp);
    }

    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "pin_mta_post_usage error", ebufp);
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
    char                *template = "select X from /mso_mta_job where F1 = V1 and F2.type = V2 ";
    char                obj_type[100];
    char                *object_type = NULL;
    int32               s_flags = 0;
    poid_t              *s_pdp = NULL;
    poid_t              *i_pdp = NULL;
    int32               i = NEW_RECORD;
    int64               id = -1;
    int64               db = 0;
    void                *vp = NULL;


    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    *s_flistpp = 0;

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", 
                       app_flistp);

    object_type = PIN_FLIST_FLD_GET(app_flistp,PIN_FLD_OBJ_TYPE,0,ebufp);
    
    if (PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_FAILED_RECORDS, 1,ebufp) )
    {
        i = ERROR_RECORD ;    
    }
    
    s_flistp = PIN_FLIST_CREATE(ebufp);

    /***********************************************************
     * Simple search flist
     ***********************************************************
     * 0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
     * 0 PIN_FLD_FLAGS           INT [0] 0
     * 0 PIN_FLD_TEMPLATE        STR [0] "select X from /mso_mta_job where F1 = V1 and F2.type = V2 "
     * 0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
     * 1     PIN_FLD_STATUS     ENUM [0] 0
     * 0 PIN_FLD_ARGS          ARRAY [2] allocated 1, used 1
     * 1     PIN_FLD_POID       POID [0] 0.0.0.1 /mso_mta_job/device/stb -1 0
     * 0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 2 
    ***********************************************************/

    vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
    if(vp)
        db = PIN_POID_GET_DB ((poid_t*)vp);

    s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
    PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, 
                      (void *)s_pdp, ebufp);

    PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
    
    /***********************************************************
     * Search arguments: Look for newly loaded records
     * Post processing status will change to success or error
     ***********************************************************/
    tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS,
                                     1, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, &i, ebufp);

    /***********************************************************
     * Search arguments: Different devices  are loaded 
     *        with different storage class poid_type
     ***********************************************************/
     i_pdp = PIN_POID_CREATE(db, object_type, id, ebufp);

     tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 2, ebufp);
     PIN_FLIST_FLD_PUT (tmp_flistp, PIN_FLD_POID, i_pdp, ebufp);


        /******************************************************************
         * Non- device type application. Eg payment, adjustment, efund etc
         *    Assumption:
         *  they will be loaded in /mso_mta_job/adjustemnet,
         ******************************************************************/
    PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_TEMPLATE, template, ebufp);

    /***********************************************************
     * Search results
     ***********************************************************/
    tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_RESULTS,0, ebufp);


    if (PIN_ERR_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "pin_mta_init_search error", ebufp);
        PIN_FLIST_DESTROY_EX (&s_flistp,0);
    }
    else
    {
        *s_flistpp = s_flistp;
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", 
                           s_flistp);
    }

    return;
}


/*******************************************************************
 * Function called when new job is avaialble to worker
 * Called prior MTA_WORKER_JOB policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_worker_job(
        pcm_context_t       *ctxp,
        pin_flist_t        *srch_res_flistp,
        pin_flist_t        **op_in_flistpp,
        pin_flist_t        *ti_flistp,
        pin_errbuf_t        *ebufp)
{

    pin_flist_t             *app_flistp = NULL;
    pin_buf_t               *nota_buf     = NULL;

    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job search results flist", 
                       srch_res_flistp);
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job opcode_in  flist",
                       *op_in_flistpp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job thread info flist", 
                       ti_flistp);

    /********************************************
    *      prepare flist for opcode call in 
    *           pin_mta_worker_opcode
    *********************************************/
    nota_buf = (pin_buf_t *)PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_INPUT_FLIST, 0, ebufp);
    PIN_STR_TO_FLIST((char *)nota_buf->data, 1, op_in_flistpp, ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
            "pin_mta_worker_job prepared flist for main opcode", *op_in_flistpp);

    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "pin_mta_worker_job error", ebufp);
    }
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
    int32                   new_status = SUCCESS_RECORD;
    int32                   new_err_status = ERROR_RECORD;
    int32                   *opcode = NULL;
    int32                   *opcode_status = NULL;
    char                    *msg[128];
    char		    *error_descr = NULL;
    char		    *error_code	= NULL;
    char		    err_code[20];
    char		    err_desc[255];
    pin_err_location_t      err_location;
    pin_err_class_t         err_class;
    pin_err_t               pin_err;
    pin_fld_num_t           err_field;
    pin_rec_id_t            err_rec_id;
    pin_err_reserved_t      err_reserved;


    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search results flist", 
                       srch_res_flistp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode", 
                       op_in_flistp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode thread info flist", 
                       ti_flistp);
    
    /*******************************************
    *     Get the opcode number to be called
    *******************************************/
    opcode = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_OPCODE,0,ebufp);
    
//    sprintf(msg, " Opcode value is %d ",*opcode);
//    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
    
    /*******************************************
    *            Execute opcode
    *******************************************/
    PCM_OP (ctxp, *opcode, 0, op_in_flistp, op_out_flistpp, ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist of opcode is  ",
                *op_out_flistpp);

    /*******************************************
    *     Update the status of the object
    *******************************************/
    if (PIN_ERR_IS_ERR(ebufp)) 
    {
        err_location = ebufp->location;
        err_class = ebufp->pin_errclass;
        pin_err = ebufp->pin_err;
        err_field = ebufp->field;
        err_rec_id = ebufp->rec_id;
	if(err_location == PIN_ERRLOC_DM)
	{
		PIN_ERR_LOG_MSG (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode DM error\n");
		sprintf(err_code,"%s","11001");
		sprintf(err_desc,"%s","Database Error");
		
	}
	else
	{
		sprintf(err_code,"%d",pin_err);
	}
        PIN_ERRBUF_CLEAR (ebufp);
        new_status = ERROR_RECORD;
    }
	
    if(*op_out_flistpp != NULL)
    {
    	error_code = PIN_FLIST_FLD_GET(*op_out_flistpp, PIN_FLD_ERROR_CODE , 1 , ebufp);
   	error_descr = PIN_FLIST_FLD_GET(*op_out_flistpp, PIN_FLD_ERROR_DESCR, 1 , ebufp);
    	opcode_status = PIN_FLIST_FLD_GET(*op_out_flistpp,PIN_FLD_STATUS , 1 , ebufp);
    }
        
    w_inflist = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(srch_res_flistp,PIN_FLD_POID,w_inflist,PIN_FLD_POID,ebufp);
    PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_STATUS, &new_status,ebufp);
    
    if (new_status == ERROR_RECORD)
    {
	
	if(strcmp(err_code,"10") == 0)
	{
		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_DESCR,"Duplicate Device ID",ebufp);
		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_CODE,"51040", ebufp);
	}
	else if (strcmp(err_code,"46") == 0)
	{
		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_DESCR," Bad Device id value " , ebufp);
		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_CODE,err_code, ebufp);
	}
	else if (strcmp(err_code,"4") == 0)
        {
                PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_DESCR," Bad value passed " , ebufp);
                PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_CODE,err_code, ebufp);
        }
	else if(strcmp(err_code , "50") == 0)
	{
		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_DESCR," Connection Error " , ebufp);
                PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_CODE,err_code, ebufp);
	}
	else if(strcmp(err_code , "11001") == 0)
	{
		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_DESCR,err_desc , ebufp);
                PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_CODE,err_code, ebufp);
	}
	else if(error_descr || err_code)
	{
		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_DESCR,"Failed in Execution", ebufp);
		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_CODE, "11001", ebufp);
	}
    }
    else
    {
	if((opcode_status) && (*opcode_status == 1) && (error_code))
	{
		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_DESCR, error_descr, ebufp);
		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_CODE, error_code, ebufp);
    		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_STATUS, &new_err_status,ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(w_inflist,PIN_FLD_ERROR_DESCR, "suceesfully completed the job ", ebufp);
	}
    }
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode write fields input flist  ",
                                     w_inflist);
    PCM_OP (ctxp,PCM_OP_WRITE_FLDS,0, w_inflist, &w_o_flist,ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode write fields output flist  ",
                                           w_o_flist);
    if (PIN_ERR_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "pin_mta_worker_opcode error", ebufp);
    }else{
            PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist from main opcode", 
                           *op_out_flistpp);
    }

    if (new_status == ERROR_RECORD)
    {
        pin_set_err(ebufp, err_location,
            err_class,
            pin_err, err_field, err_rec_id, 0);
    }

    PIN_FLIST_DESTROY_EX(&w_inflist , NULL);
    PIN_FLIST_DESTROY_EX (&w_o_flist, NULL);
    return;
}
