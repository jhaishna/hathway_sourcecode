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
op_mso_utils_create_xml_buffer(
        cm_nap_connection_t	*connp,
		int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t	*ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_CREATE_CATV_ACTION command
 *******************************************************************/
void
op_mso_utils_create_xml_buffer(
        cm_nap_connection_t	*connp,
		int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t	*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
        pin_flist_t		*tmp_flistp = NULL;
        pin_flist_t		*r_flistp = NULL;
	mso_prov_str_buf_t		dbuf;
	pin_buf_t		*nota_buf     = NULL;
        dbuf.strp = NULL;
    	dbuf.strsize = 0;
		char *action = NULL;


	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_UTILS_CREATE_XML_BUF ) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_mso_utils_create_xml_buffer", ebufp);
		return;
	}

	/***********************************************************
	 * Log input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_utils_create_xml_buffer input flist", in_flistp);

        /*
         * Convert flist to XML format. it will be used by Provising system.
         */
	action = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACTION, 0, ebufp);
	tmp_flistp = PIN_FLIST_COPY(in_flistp, ebufp);
	PIN_FLIST_FLD_DROP(tmp_flistp, PIN_FLD_ACTION, ebufp);
	PIN_FLIST_FLD_DROP(tmp_flistp, PIN_FLD_POID, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_utils_create_xml_buffer after field drop flist", tmp_flistp);
        PIN_FLIST_TO_XML (tmp_flistp, PIN_XML_BY_SHORT_NAME, 0, &dbuf.strp, &dbuf.strsize, action, ebufp );


        nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
    if ( nota_buf == NULL )
        {
             pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
             PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate nemory for nota_buf", ebufp );
             return;
        }

        nota_buf->flag   = 0;
    nota_buf->size   = dbuf.strsize - 1;
    nota_buf->offset = 0;
    nota_buf->data   = ( void *) dbuf.strp;
    nota_buf->xbuf_file = ( char *) NULL;

PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
		"before object creation");

r_flistp = PIN_FLIST_CREATE(ebufp);
PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"at 1");
        PIN_FLIST_FLD_COPY( in_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID,  ebufp );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"at 2");
        PIN_FLIST_FLD_PUT( r_flistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_utils_create_xml_buffer create obj input flist", r_flistp);

	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, r_flistp, ret_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_utils_create_xml_buffer create obj return flist", *ret_flistpp);



PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_utils_create_xml_buffer error", ebufp);
	} else {
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_utils_create_xml_buffer return flist", *ret_flistpp);
	}

	return;
}
