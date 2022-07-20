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
static  char    Sccs_id[] = "@(#)%Portal Version: mso_update_credit_note_gst_flag.c:CUPmod7.3PatchInt:1:2007-Feb-07 06:51:33 %";
#endif

#include <stdio.h>
#include "pcm.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_mta.h"
#include "pin_flds.h"
#include "ops/base.h"
#include "mso_ops_flds.h"
#include "ops/bill.h"


/*******************************************************************
* Apllication constants
********************************************************************/
#define  MODE_VERBOSE		1
#define  MODE_NON_VERBOSE	0

/*******************************************************************
* GST FLAG function
*******************************************************************/
int getGSTINflag(
        pcm_context_t   *ctxp,
        char            *bill_no,
        pin_errbuf_t    *ebufp);

/*******************************************************************
* Status update function
*******************************************************************/
void StatusUpdate(
	pcm_context_t *ctxp, 
	poid_t *pdp ,
	int32 s_flag, 
	pin_errbuf_t *ebufp);


/*******************************************************************
 * Configuration of application
 * Called prior MTA_CONFIG policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_config(
	pin_flist_t	*param_flistp,
	pin_flist_t	*app_flistp,
	pin_errbuf_t	*ebufp)
{
	void		*vp = 0;
	int32		mta_flags = 0;
	time_t		start_t=(time_t )0;
	char		*time_value=NULL;
        char                    *option = 0;
        int32                   i_param_count = 0;
	int32                   rec_id = 0;
        pin_cookie_t    cookie = 0;
        pin_cookie_t    prev_cookie = 0;
        pin_flist_t             *flistp = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config parameters flist",
		param_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config application info flist",
		app_flistp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FLAGS, 0,ebufp);
	if(vp)
		mta_flags = *((int32*)vp);
	
	mta_flags = mta_flags | MTA_FLAG_VERSION_NEW;
	PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);


	if (PIN_ERRBUF_IS_ERR(ebufp)) {
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
char	*prog)
{

	pin_errbuf_t	ebuf;
	pin_flist_t	*ext_flistp = NULL;
	char		*usage_str = NULL;

	char            *format = "\nUsage \t %s [-v] \n"
			"\t\t -h help option \n"
			"\t\t Example: mso_update_credit_note_gst_flag \n";

	PIN_ERRBUF_CLEAR (&ebuf);

	usage_str = (char*)pin_malloc( strlen(format) + strlen(prog) + 1 );

	if (usage_str == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No Memory error");
		return;
	}

	sprintf(usage_str, format ,prog);
	//fprintf( stderr,usage_str);

	ext_flistp =pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO, &ebuf);

	PIN_FLIST_FLD_SET (ext_flistp, PIN_FLD_DESCR, usage_str, &ebuf)

	if (PIN_ERRBUF_IS_ERR(&ebuf)) {
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
	pin_flist_t	*param_flistp,
	pin_flist_t	*app_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*ext_flistp = NULL;
	void		*vp = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage parameters flist", 
		param_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage application info flist", 
		app_flistp);

	ext_flistp = pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO, ebufp);

	vp = PIN_FLIST_FLD_GET (ext_flistp, PIN_FLD_DESCR, 1, ebufp);

	if(vp) {
		printf("%s",(char*) vp);
	}

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
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
	pin_flist_t	*app_flistp,
	pin_flist_t	**s_flistpp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	*s_flistp = NULL;
	pin_flist_t 	*tmp_flistp=NULL;
        pin_flist_t     *l_flistp=NULL;
        pin_flist_t     *x_flistp=NULL;
	int32		s_flags = 512;
	poid_t		*s_pdp = NULL;
        poid_t          *pdp = NULL;
	int64		db = 0;
	void		*vp = NULL;
	int32    	*db_list = 0;
 	char            ptemplate[200] = "select X from /mso_adjustment 1,/temp_mso_adj 2 where 2.F3 = V3 and 1.F1 = 2.F2 ";
        int64                   id1 = 1;
        int32           link_direction=1;
        int32           stat_flag = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {	
		PIN_ERRBUF_RESET(ebufp);	
		return;	
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist",  app_flistp);

	*s_flistpp = 0;
	s_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
	if(vp)
		db = PIN_POID_GET_DB ((poid_t*)vp);
		
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"flags error", ebufp);
                PIN_FLIST_DESTROY_EX (&s_flistp,0);
        }

	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID,NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PARENT,NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS_FLAGS,&stat_flag, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"args error", ebufp);
                PIN_FLIST_DESTROY_EX (&s_flistp,0);
        }

 	PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_TEMPLATE, ptemplate, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"result error", ebufp);
                PIN_FLIST_DESTROY_EX (&s_flistp,0);
        }
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID,NULL, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_BILL_NO,NULL, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"result set error", ebufp);
                PIN_FLIST_DESTROY_EX (&s_flistp,0);
        }
	
	l_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_LINKED_OBJ, 2, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"linkobj error", ebufp);
                PIN_FLIST_DESTROY_EX (&s_flistp,0);
        }

        PIN_FLIST_FLD_SET(l_flistp,PIN_FLD_LINK_DIRECTION, (void*)&link_direction, ebufp);
        PIN_FLIST_FLD_SET(l_flistp,PIN_FLD_PARENT, NULL, ebufp);
	
	x_flistp = PIN_FLIST_ELEM_ADD(l_flistp, PIN_FLD_EXTRA_RESULTS, -1, ebufp);
        //PIN_FLIST_FLD_SET(x_flistp,PIN_FLD_POID, NULL, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"extra error", ebufp);
                PIN_FLIST_DESTROY_EX (&s_flistp,0);
        }
        PIN_FLIST_FLD_SET(x_flistp,PIN_FLD_PARENT, NULL, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"pin_mta_init_search error", ebufp);
		PIN_FLIST_DESTROY_EX (&s_flistp,0);

	}else
	{
		*s_flistpp = s_flistp;
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", s_flistp);
	}


    return;
}

/*******************************************************************
 * Search results are modified to consider extra outouts into worker opcode
 * Called after MTA_TUNE policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_post_tune(
                pin_flist_t             *app_flistp,
                pin_flist_t             *srch_res_flistp,
                pin_errbuf_t    *ebufp)
{
        pin_flist_t             *rest_flistp=NULL;
        pin_flist_t             *multirest_flistp=NULL;
        pin_cookie_t            cookie = NULL;
        int                     elem_id = 0;
        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR (ebufp);

        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune application info flist",
                                           app_flistp);
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results flist",
                                           srch_res_flistp);
        multirest_flistp = PIN_FLIST_ELEM_GET(srch_res_flistp,PIN_FLD_MULTI_RESULTS,PIN_ELEMID_ANY,1,ebufp);
        while((rest_flistp = PIN_FLIST_ELEM_GET_NEXT(multirest_flistp, PIN_FLD_RESULTS,
                &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
                PIN_FLIST_ELEM_COPY(multirest_flistp, PIN_FLD_EXTRA_RESULTS, elem_id + 1, rest_flistp, PIN_FLD_EXTRA_RESULTS, 0, ebufp);
                PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results modify flist", rest_flistp);
        }
	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "pin_mta_post_tune error", ebufp);
        }
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results output flist", srch_res_flistp);

        return;
}

/*******************************************************************
 * Function called when new job is avaialble to worker
 * Called prior MTA_WORKER_JOB policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_worker_job(
	pcm_context_t	*ctxp,
	pin_flist_t	*srch_res_flistp,
	pin_flist_t	**op_in_flistpp,
	pin_flist_t	*ti_flistp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	*app_flistp = NULL;
	pin_buf_t	*nota_buf = NULL;
	void		*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job search results flist", 
	       srch_res_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job opcode_in  flist",
	       *op_in_flistpp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job thread info flist", 
	       ti_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
	"pin_mta_worker_job prepared flist for main opcode", *op_in_flistpp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
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
	pcm_context_t	*ctxp,
	pin_flist_t	*srch_res_flistp,
	pin_flist_t	*op_in_flistp,
	pin_flist_t	**op_out_flistpp,
	pin_flist_t	*ti_flistp,
	pin_errbuf_t	*ebufp)
	{
        pin_flist_t     *extrars_flistp = NULL;
	poid_t 		*tmp_pdp=NULL;
	pin_flist_t	*wf_in_flistp = NULL;
        pin_flist_t     *wf_out_flistp = NULL;
        int32            gst_flag = 0;
        char                    *bill_no = NULL;
        char                    msg[256] = "";
	poid_t		*pdp=NULL;
	int32		status=0;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);


	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode start.");
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search results flist", 
		srch_res_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode", 
		op_in_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode thread info flist", 
		ti_flistp);


        pdp=PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_POID, 0, ebufp);

	if(pdp){
		bill_no=PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_BILL_NO,0, ebufp);
                extrars_flistp = PIN_FLIST_ELEM_GET(srch_res_flistp,PIN_FLD_EXTRA_RESULTS,0,0,ebufp);
                tmp_pdp=PIN_FLIST_FLD_TAKE(extrars_flistp, PIN_FLD_POID,0, ebufp);
	}
	if (bill_no == NULL){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Bill number is null.");
		goto CLEANUP;
	}
  
	wf_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(wf_in_flistp, PIN_FLD_POID, pdp, ebufp);

        /**Adding GSTIN Flag in /mso_adjustement START */

       	gst_flag = getGSTINflag(ctxp, bill_no, ebufp);
        sprintf(msg, "gst_flag : %d",gst_flag);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
        PIN_FLIST_FLD_SET(wf_in_flistp,PIN_FLD_REASON_ID, &gst_flag, ebufp);

        /**Adding GSTIN Flag in /mso_adjustement END */

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " write reason_id into mso_adjustment input flist:", wf_in_flistp);

        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wf_in_flistp, &wf_out_flistp,ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " write reason_id into mso_adjustment output flist:", wf_out_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp)){
       	        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in wriring fields to mso_adjustment.",ebufp);
		status=2;

		goto CLEANUP;
	}

	status =1;

	*op_out_flistpp = PIN_FLIST_COPY(wf_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, " pin_mta_worker_opcode output flist",*op_out_flistpp);


	if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "pin_mta_worker_job error", ebufp);
        }
	 CLEANUP:
                PIN_FLIST_DESTROY_EX(&wf_in_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&wf_out_flistp, NULL);

	/*Function to update status in /temp_mso_adj object*/
        StatusUpdate(ctxp, tmp_pdp, status, ebufp);


	if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "pin_mta_worker_job error", ebufp);
        }

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode end.");
	return;
}

int getGSTINflag(pcm_context_t *ctxp, char *bill_no, pin_errbuf_t *ebufp) {

        char *p, *temp, *gst_temp, *gst_temp2;
        int na_flag = 0;
        int gst_flag = 0;

        int32           flags = 512;

        poid_t          *s_pdp = NULL;
        poid_t          *p_pdp = NULL;

        pin_flist_t     *srch_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *ss_flistp = NULL;
        pin_flist_t     *p_flistp = NULL;
        pin_flist_t     *buf_flistp = NULL;
        pin_flist_t     *r_s_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;

        pin_buf_t       *pin_bufp = NULL;
	 char            *template =  "select X from /invoice where F1 = V1 ";
        char            debugmsg[100];

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp  = PIN_POID_FROM_STR("0.0.0.1 /search -1 0", NULL, ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp,PIN_FLD_POID, s_pdp, ebufp);

        PIN_FLIST_FLD_SET(srch_flistp,PIN_FLD_TEMPLATE, template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_NO, bill_no, ebufp);
        sprintf(debugmsg,"Bill_no- %s",bill_no);
        PIN_ERR_LOG_MSG(3,debugmsg);
        PIN_FLIST_FLD_SET(srch_flistp,PIN_FLD_FLAGS, (void*)&flags, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(3, "invoice format search input flistp", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &r_s_flistp, ebufp);
        PIN_ERR_LOG_FLIST(3, "invoice format search output flistp", r_s_flistp);

        if (PIN_FLIST_ELEM_COUNT(r_s_flistp, PIN_FLD_RESULTS, ebufp) > 0) {
                r_flistp = PIN_FLIST_ELEM_GET( r_s_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
                buf_flistp = PIN_FLIST_ELEM_GET( r_flistp, PIN_FLD_FORMATS, 0, 0, ebufp );
                pin_bufp = (pin_buf_t *)PIN_FLIST_FLD_GET( buf_flistp, PIN_FLD_BUFFER, 0, ebufp );
	
		 /*Extract the GSTIN from the Invoice Buffer String */
                p = strtok_r(pin_bufp->data, "\n", &temp);
                do {
                        if( strstr(p, "MSO_FLD_GSTIN") ) {
				gst_temp = p + 16;
                                if(gst_temp != NULL && *gst_temp != '<') {
        	                        gst_temp2 = strstr(gst_temp, "</M");
                	                gst_temp2 = '\0';
                        	        gst_flag = (strlen(gst_temp) > 0) ? 1 : 0 ;
                       		}
				sprintf(debugmsg,"GSTIN-%s",gst_temp);
                        	PIN_ERR_LOG_MSG(3,debugmsg);
                        	break;
			}
                } while ((p = strtok_r(NULL, "\n", &temp)) != NULL);

        }

       PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

       PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);

        return gst_flag;
}

void StatusUpdate(pcm_context_t *ctxp, poid_t *pdp ,int32 s_flag, pin_errbuf_t *ebufp){
	pin_flist_t     *status_win_flistp = NULL;
        pin_flist_t     *status_wout_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in status update.",ebufp);
                return ;
        }

	status_win_flistp= PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(status_win_flistp, PIN_FLD_POID, pdp, ebufp);
        PIN_FLIST_FLD_SET(status_win_flistp, PIN_FLD_STATUS_FLAGS, &s_flag, ebufp);

        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, status_win_flistp, &status_wout_flistp,ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " write status_flag into temp_mso_adj output flist:", status_wout_flistp);

        if (PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in wriring fields to mso_adjustment.",ebufp);
        }
        
        PIN_FLIST_DESTROY_EX(&status_win_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&status_wout_flistp, NULL);


}
