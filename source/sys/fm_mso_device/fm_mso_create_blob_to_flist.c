/*******************************************************************
 *
 * Copyright (c) 1999, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

 /***************************************************************************************************************
*VERSION |   DATE    |    Author        |               DESCRIPTION                                            *
*--------------------------------------------------------------------------------------------------------------*
* 0.1    |25/10/2013 |Satya Prakash     |  CATV Provisioning implementation 
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_prov_create_catv_action.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/timeb.h>
#include "pin_sys.h"
#include "pin_os.h"
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_prov.h"
#include "cm_cache.h"


/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_utils_blob_to_flist(
        cm_nap_connection_t	*connp,
		int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t	*ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_CREATE_CATV_ACTION command
 *******************************************************************/
void
op_mso_utils_blob_to_flist(
        cm_nap_connection_t	*connp,
		int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t	*ebufp)
{

        pcm_context_t           *ctxp = connp->dm_ctx;
        pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *buf_to_flistp = NULL;
        pin_flist_t     *result_flist = NULL;
        pin_flist_t     *tmp_flistp = NULL;
        pin_buf_t       *pin_bufp     = NULL;
        int64            database=0;
        int32           *failed_task=NULL;
        poid_t          *s_pdp     =NULL;
        poid_t          *pdp     =NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        /***********************************************************
         * Null out results
         ***********************************************************/
        *r_flistpp = NULL;

 

	if (opcode != MSO_OP_UTILS_BLOB_TO_FLIST ) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_mso_utils_blob_to_flist", ebufp);
		return;
	}
         
         pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
         database = PIN_POID_GET_DB(pdp);
         /***********************************************************
         * Debut what we got.
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_device_pol_test input flist", i_flistp);

        /***********************************************************

         * Main rountine for this opcode
         ***********************************************************/

        tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
        

        s_pdp  = (poid_t *) PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp );
        pin_bufp = (pin_buf_t *) PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_INPUT_FLIST, 0, ebufp );
        failed_task = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TASK_FAILED, 0, ebufp );
        if (pin_bufp)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pin_bufp->data);
        }
                /* Convert the buffer to a flist */
        PIN_STR_TO_FLIST((char *)pin_bufp->data, 1, &r_flistp, ebufp);

         PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "buffer to flist converted flist output flist", r_flistp);
        /***********************************************************
         * Error?
         ***********************************************************/
        buf_to_flistp = PIN_FLIST_CREATE(ebufp);  
        PIN_FLIST_FLD_SET(buf_to_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(buf_to_flistp, MSO_FLD_TASK_FAILED,failed_task, ebufp);
        result_flist = PIN_FLIST_ELEM_ADD(buf_to_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "buffer to flist converted flist output flist", buf_to_flistp);

        PIN_FLIST_CONCAT(result_flist,r_flistp, ebufp); 

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "buffer to after elem flist converted flist output flist", buf_to_flistp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_FLIST_DESTROY_EX(r_flistpp, ebufp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_device_pol_test error", ebufp);
        } else {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_device_pol_test output flist",buf_to_flistp);
                 }
         PIN_ERRBUF_CLEAR(ebufp);
         

        *r_flistpp = PIN_FLIST_COPY(buf_to_flistp,ebufp);
	PIN_FLIST_DESTROY_EX(&buf_to_flistp,NULL);
        return;

}
