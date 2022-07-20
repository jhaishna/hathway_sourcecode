/*******************************************************************************
*
*      Copyright (c) 2010 Oracle Corp. All rights reserved.
*
*      This material is the confidential property of Oracle Corporation
*      or its licensors and may be used, reproduced, stored or transmitted
*      only in accordance with a valid Oracle license or sublicense agreement.
********************************************************************************/

#ifndef lint
   static const char rcsid[] = "@(#) $Id: fm_mso_utils.c,v 1.2 2011/03/03 14:25:20  Exp $";
#endif


#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

#include "pcm.h"
#include "cm_fm.h"
#include "cm_cache.h"
#include "pin_errs.h"
#include "pinlog.h"


/**************************************************
* Transaction management functions
***************************************************/
int
fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*tr_flistp = NULL;
	pin_flist_t	*tar_flistp = NULL;

	int		t_status = 0;

	if (  PIN_ERRBUF_IS_ERR( ebufp ) )
    {
      PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_ERROR, "fm_trans_open : Error while openning a transaction" );
	  return 0;	
	}
   

        t_flistp = PIN_FLIST_CREATE(ebufp);
//        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, (void *)pdp, ebufp );
   		PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID,(void *)pdp, ebufp );
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "pdp for transaction in trans_open",pdp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_trans_open open flist", t_flistp );
	/* Open transaction */
        PCM_OP(ctxp, PCM_OP_TRANS_OPEN, PCM_TRANS_OPEN_READWRITE, t_flistp, &tr_flistp, ebufp);

        if(ebufp->pin_err == PIN_ERR_TRANS_ALREADY_OPEN)
        {
                PIN_ERRBUF_CLEAR(ebufp);
		t_status = 1;
        }
        else if(ebufp->pin_err != PIN_ERR_NONE)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "fm_mso_trans_open trans open error", ebufp);
		t_status = 1;
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_trans_open trans open");
	}

	/* cleanup */
	if(t_flistp)
		PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	if(tr_flistp)
		PIN_FLIST_DESTROY_EX(&tr_flistp, NULL);
	if(tar_flistp)
		PIN_FLIST_DESTROY_EX(&tar_flistp, NULL);

	return t_status;
}


void
fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*tr_flistp = NULL;

 	if (  PIN_ERRBUF_IS_ERR( ebufp ) )
    {
      PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_ERROR, "fm_mso_trans_commit : Error while openning a transaction" );
	  return;	
	}

        t_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_trans_commit commit flist", t_flistp );
	/* Commit transaction */
        PCM_OP(ctxp, PCM_OP_TRANS_COMMIT, 0, t_flistp, &tr_flistp, ebufp);
		  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_trans_commit commit return flist", tr_flistp );
        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "fm_mso_trans_commit trans commit error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_trans_commit trans commit");
	}

	/* cleanup */
	if(t_flistp)
		PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	if(tr_flistp)
		PIN_FLIST_DESTROY_EX(&tr_flistp, NULL);

	return;
}


void
fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*tr_flistp = NULL;

	  if (  PIN_ERRBUF_IS_ERR( ebufp ) )
    {
      PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_ERROR, "fm_mso_trans_abort : Error while openning a transaction" );
	  return;	
	}
        t_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_trans_abort abort flist", t_flistp );
	/* Open transaction */
        PCM_OP(ctxp, PCM_OP_TRANS_ABORT, 0, t_flistp, &tr_flistp, ebufp);

        if(ebufp->pin_err != PIN_ERR_NONE)
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "fm_mso_trans_abort trans abort error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_trans_abort trans abort");
	}

	/* cleanup */
	if(t_flistp)
		PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	if(tr_flistp)
		PIN_FLIST_DESTROY_EX(&tr_flistp, NULL);

	return;
}
