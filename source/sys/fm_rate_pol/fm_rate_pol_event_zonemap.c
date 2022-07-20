/*******************************************************************
 *
 *      Copyright (c) 2004 - 2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its subsidiaries or licensors and may be used, reproduced, stored
 *      or transmitted only in accordance with a valid Oracle license or
 *      sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_rate_pol_event_zonemap.c:IDCmod7.3.1Int:1:2007-Dec-11 11:42:45 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_RATE_POL_EVENT_ZONEMAP operation. 
 * Used for common zoning 
 * From the input flist which is a combination of event flist and zonemap name,
 * this opcode extracts the service type, calling number, called number , 
 * zonemap name. Sends the flist with these parameters to the realtime pipeline and 
 * recieve the pipe line impact category.
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"

#define FILE_SOURCE_ID  "fm_rate_pol_zone.c"

#define SERVICE_TELEPHONY	"/service/telephony"
#define SERVICE_GPRS		"/service/gprs"
#define SERVICE_TELCO		"/service/telco"
#define SERVICE_GSM		"/service/gsm"
#define SERVICE_WAP		"/service/wap"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_rate_pol_event_zonemap(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);


void
op_rate_pol_event_zonemap(
        cm_nap_connection_t     *connp,
        u_int                   opcode,
        u_int                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{
	
	pin_flist_t            *evt_flistp = NULL;
        pin_flist_t            *sub_flistp = NULL;
        pin_flist_t            *column_flistp = NULL;
        pin_flist_t            *ret_flistp = NULL;
        int32                   rec_id = 0;
        poid_t                  *s_pdp;
        poid_t                  *evt_pdp;
        time_t                  *start_t =  NULL;
        time_t                  *end_t =  NULL;
        char                    *a_num = NULL;
        char                    *b_num = NULL;
        int64                   dbno;
        long                    now=0;
        char                    *svc;
	char			*zone;
	pcm_context_t           *ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	
	if (opcode != PCM_OP_RATE_POL_EVENT_ZONEMAP) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_rate_pol_event_zonemap", ebufp);
		return;
	}

	evt_pdp = PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_POID, 0, ebufp);
	s_pdp = PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
	svc = (char *) PIN_POID_GET_TYPE(s_pdp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"In op_rate_pol_get_impact_category..");

	if(svc && !strncmp(svc,SERVICE_TELEPHONY,strlen(SERVICE_TELEPHONY)))
	{
	column_flistp = PIN_FLIST_ELEM_GET(i_flistp,PIN_FLD_CALL, rec_id, 1, ebufp);
	a_num = PIN_FLIST_FLD_GET( column_flistp, PIN_FLD_ANI, 1, ebufp );
	b_num = PIN_FLIST_FLD_GET( column_flistp, PIN_FLD_DNIS, 1, ebufp );
	}

	else if(svc && !strncmp(svc,SERVICE_GPRS,strlen(SERVICE_GPRS)))
	{
	column_flistp = PIN_FLIST_ELEM_GET(i_flistp,PIN_FLD_GPRS_INFO, rec_id,
						1, ebufp);
	a_num = PIN_FLIST_FLD_GET( column_flistp,PIN_FLD_ANI, 1, ebufp );
	b_num = PIN_FLIST_FLD_GET( column_flistp,PIN_FLD_DN, 1, ebufp );
	}

	else if(svc && !strncmp(svc,SERVICE_TELCO,strlen(SERVICE_TELCO)))
	{
	column_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TELCO_INFO, rec_id,
						1, ebufp);
	a_num = PIN_FLIST_FLD_GET( column_flistp, PIN_FLD_CALLING_FROM, 1, ebufp );
	b_num = PIN_FLIST_FLD_GET( column_flistp, PIN_FLD_CALLED_TO, 1, ebufp );
	}

	else if(svc && !strncmp(svc,SERVICE_GSM,strlen(SERVICE_GSM)))
	{
	column_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_GSM_INFO, rec_id,
						1, ebufp);
	a_num = PIN_FLIST_FLD_GET( column_flistp,PIN_FLD_CALLING_NUMBER, 1, ebufp );
	b_num = PIN_FLIST_FLD_GET( column_flistp,PIN_FLD_CALLED_NUMBER, 1, ebufp );
	}

	else if(svc && !strncmp(svc,SERVICE_WAP,strlen(SERVICE_WAP)))
	{
	column_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_WAP_INFO, rec_id,
						1, ebufp);
	a_num = PIN_FLIST_FLD_GET( column_flistp,PIN_FLD_ANI, 1, ebufp );
	b_num = PIN_FLIST_FLD_GET( column_flistp,PIN_FLD_DN, 1, ebufp );
	}

	start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp );
	end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp );
	zone = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ZONEMAP_NAME, 1, ebufp);

	if( a_num == (char *) NULL || b_num == (char *) NULL || 
						zone == (char *) NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,"Null Value in one or "
				"more of : A Number, B Number and Zone Map Name");
		return;
	}

	evt_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(evt_flistp, PIN_FLD_POID, (void *)evt_pdp, ebufp);

	sub_flistp = PIN_FLIST_SUBSTR_ADD(evt_flistp, PIN_FLD_EVENT, ebufp);
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_POID, (void *)evt_pdp, ebufp);
	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SERVICE_OBJ, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_START_T, (void *)start_t, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_END_T, (void *)end_t, ebufp);
	
	PIN_FLIST_FLD_SET(evt_flistp, PIN_FLD_SERVICE_CLASS, (void *)svc, ebufp);
	PIN_FLIST_FLD_SET(evt_flistp, PIN_FLD_ANI, (void *)a_num, ebufp);
        PIN_FLIST_FLD_SET(evt_flistp, PIN_FLD_DN, (void *)b_num, ebufp);
	PIN_FLIST_FLD_SET(evt_flistp, PIN_FLD_ZONEMAP_NAME, (void *)zone, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"PCM_OP_RATE_GET_ZONEMAP "
					"i/p flist :",evt_flistp);

        PCM_OP(&ctxp, PCM_OP_RATE_GET_ZONEMAP_INFO, 0, evt_flistp, 
        						&ret_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&evt_flistp, NULL);
	
	if (ret_flistp == (pin_flist_t *)NULL) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"return flist from "
                         "PCM_OP_RATE_GET_ZONEMAP is null");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_event_zonemap error", ebufp);
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"PCM_OP_RATE_GET_ZONEMAP "
					"o/p flist :",ret_flistp);

	*o_flistpp = ret_flistp;

	if(PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
			"op_rate_pol_event_zonemap return error");
		*o_flistpp = NULL;
	}
	return;
}
