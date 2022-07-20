/**********************************************************************
 *
* Copyright (c) 2004, 2014, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its subsidiaries or licensors and may be used, reproduced, stored
 *      or transmitted only in accordance with a valid Oracle license or
 *      sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)$Id: fm_rate_pol_process_eras.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 04:13:43 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_RATE_POL_PROCESS_ERAS operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 

#include "pcm.h"
#include "ops/rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_os_string.h"
#include "pinlog.h"
#include "pin_bill.h"

#define FILE_SOURCE_ID  "fm_rate_pol_process_eras.c(20)"

#define SERVICE_TELEPHONY	"/service/telephony"
#define SERVICE_GPRS		"/service/ip/gprs"
#define SERVICE_TELCO		"/service/telco"

#define	FF		0x01
#define	CUG		0x10

#define FRIENDS_FAMILY		"FRIENDS_FAMILY"
#define CLOSED_USERGROUP	"CLOSEDUSERGROUP"


typedef struct node {
   char		label[125];
   struct node	*next;
} node;



PIN_IMPORT char fm_rate_evt_str_for_start_t[];

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_rate_pol_process_eras(
	cm_nap_connection_t	*connp,
	u_int			opcode,
	u_int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_rate_pol_proc_eras_set_era_name(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*p_flistp,	
	pin_flist_t	**r_flistp,
	pin_errbuf_t	*ebufp);

static void
fm_rate_pol_proc_eras_get_b_account_eras(
	pcm_context_t	*ctxp,
	pin_flist_t     *e_flistp,
	char		*service_type,
	char		*b_num,
	pin_flist_t	**prof_flistp,
	pin_errbuf_t    *ebufp);

static void
fm_rate_pol_proc_eras_get_cug_era(
	pin_flist_t     *p_flistp,
	char		*a_cug_value,
	int		*cug_flag,
	pin_errbuf_t    *ebufp);

static void
fm_rate_pol_process_eras_set_profile_name (
	pcm_context_t	*ctxp,
	pin_flist_t	*profile_flistp,
	pin_flist_t	*e_flistp,
	char		*service_type,
	char		*matching_criteria, 
	int32		*era_flag,
	node		**head,
	pin_errbuf_t	*ebufp); 

static void
fm_rate_pol_process_eras_populate_usagetype_label(
	pcm_context_t	*ctxp,
	pin_flist_t	*profile_data_flistp,
	pin_flist_t	*e_flistp,
	char		*service_type,
	char		*matching_criteria,   
	char		*profile_name,
	int32		*era_flag,
	node		**head,
	pin_errbuf_t	*ebufp);

static int32 
fm_rate_pol_proc_eras_value_match(
	pcm_context_t	*ctxp,
	pin_flist_t	*e_flistp,
	char		*profile_name, 
	char		*era_value, 
	char		*matching_criteria,
	char		*service_type,
	int32		*era_flag,
	pin_errbuf_t	*ebufp);

node* 
initnode( 
	char		*label );

void	
insertnode( 
	node		**head,
	node 		*newNodep );

void
freenode(
	node            *head);

int32
prefix_and_suffix_match(
	char		*source,
	char		*dest);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_RATE_POL_PROCESS_ERAS operation.
 *******************************************************************/
void
op_rate_pol_process_eras(
	cm_nap_connection_t	*connp,
	u_int			opcode,
	u_int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*p_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*e_flistp = NULL;
	pin_flist_t		*eras_inp_flistp = NULL;
	time_t			*evt_startt = NULL;
	time_t			*evt_endt = NULL;
	time_t			*effective_t = NULL;
	poid_t			*e_pdp= NULL;
	poid_t			*a_pdp = NULL;
	poid_t			*s_pdp = NULL;
	void			*vp = NULL;
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}

	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_RATE_POL_PROCESS_ERAS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_process_eras opcode error", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_rate_pol_process_eras input flist", i_flistp);
	
	e_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EVENT, 0, ebufp);

	e_pdp = (poid_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_POID, 0, ebufp);
	evt_startt = (time_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_START_T, 0, ebufp);
	evt_endt = (time_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_END_T, 0, ebufp);
	a_pdp = (poid_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	s_pdp = (poid_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	effective_t = (time_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp);

	eras_inp_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_POID, e_pdp, ebufp);
	PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
	PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_SERVICE_OBJ, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_START_T, evt_startt, ebufp);
	PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_END_T, evt_endt, ebufp);
	if(effective_t && *effective_t) {
		PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_EFFECTIVE_T, effective_t, ebufp);
	}
	vp =  PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_BILLINFO_OBJ, 1, ebufp);
	if (vp) {
		PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_BILLINFO_OBJ, vp, ebufp);

	}

	PCM_OPREF(ctxp, PCM_OP_RATE_GET_ERAS, 0, eras_inp_flistp, &p_flistp, ebufp);

	fm_rate_pol_proc_eras_set_era_name(ctxp, i_flistp, p_flistp, &r_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_process_eras error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;
	} else {
		*o_flistpp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_rate_pol_process_eras return flist", *o_flistpp);
	}
	
	PIN_FLIST_DESTROY_EX(&p_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&eras_inp_flistp, NULL);

	return;
}

static void
fm_rate_pol_proc_eras_set_era_name(
	pcm_context_t	*ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     *p_flistp,
        pin_flist_t     **r_flistp,
        pin_errbuf_t    *ebufp)
{
	
	pin_flist_t		*column_flistp = NULL;
	pin_flist_t		*e_flistp = NULL;
	pin_flist_t		*inherited_info_flistp = NULL;
	int32			rec_id = 0;
	int32			*era_flag = 0;
	poid_t			*s_pdp = NULL;
	poid_t			*e_pdp = NULL;
	char			*b_num = NULL;	
	char			*svc = NULL;
	char			usage_type[128] = "";
	char			label[512]="";
	node			*head = NULL;
	node                    *temp = NULL;
	int32 			max_size;
	void                    *vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Enter fm_rate_pol_proc_eras_set_era_name: ", i_flistp );

	e_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EVENT, 0, ebufp);
	s_pdp = (poid_t *) PIN_FLIST_FLD_GET( e_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	
	if(!PIN_POID_IS_NULL(s_pdp)) {
		svc = (char *) PIN_POID_GET_TYPE(s_pdp);
	}	
	era_flag = (int32*)pin_malloc(sizeof(int32));
	if (!era_flag) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NO_MEM, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_rate_pol_proc_eras_set_era_name "
			"pin_malloc for era_flag failed", ebufp);
		return;
	}

	memset(era_flag, 0, sizeof(int32));

	if(svc) {	
		if(!strncmp(svc,SERVICE_TELEPHONY,strlen(SERVICE_TELEPHONY))) {
			column_flistp = PIN_FLIST_ELEM_GET(e_flistp,PIN_FLD_CALL, rec_id, 1, ebufp);
			if(column_flistp) {
				b_num = PIN_FLIST_FLD_GET( column_flistp, PIN_FLD_DNIS, 1, ebufp );
			}
		} else if(!strncmp(svc,SERVICE_GPRS,strlen(SERVICE_GPRS))) {
			column_flistp = PIN_FLIST_ELEM_GET(e_flistp,PIN_FLD_GPRS_INFO, rec_id,
                                                1, ebufp);
			if(column_flistp) {
				b_num = PIN_FLIST_FLD_GET( column_flistp,PIN_FLD_DN, 1, ebufp );
			}
		} else if(!strncmp(svc,SERVICE_TELCO,strlen(SERVICE_TELCO))) {
			column_flistp = PIN_FLIST_ELEM_GET(e_flistp, PIN_FLD_TELCO_INFO, rec_id,
                                                1, ebufp);
			if(column_flistp) {
				b_num = PIN_FLIST_FLD_GET( column_flistp, PIN_FLD_CALLED_TO, 1, ebufp );
			}
		}
		
		if(b_num) {
			fm_rate_pol_process_eras_set_profile_name(ctxp,p_flistp, e_flistp, 
						svc, b_num, era_flag, &head, ebufp); 
		}
	}
	
	
	switch(*era_flag) {
		case FF: 
			strcpy(usage_type, "FF");
			break;
		case CUG:
			strcpy(usage_type, "CUG");
			break;
		case FF|CUG:
			strcpy(usage_type, "FF_CUG");
			break;
		default:
			break;
	}

	*r_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	e_flistp  = PIN_FLIST_SUBSTR_GET(*r_flistp, PIN_FLD_EVENT, 0, ebufp); 
	e_pdp     = (poid_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_POID, 0, ebufp);

	inherited_info_flistp = PIN_FLIST_SUBSTR_GET(*r_flistp,
		PIN_FLD_INHERITED_INFO, 1, ebufp); 
	if (inherited_info_flistp) {
		p_flistp = PIN_FLIST_ELEM_GET(inherited_info_flistp, 
			PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 1, ebufp);
	} 
	
	/* Set the PIN_FLD_USAGE_TYPE field in PIN_FLD_EVENT 
	 * only if a valid ERA is found; Otherwise, populate it 
	 * with PRICE_LIST_NAME (on product offering passed in 
	 * input) for cycle events.
	 */
	if((*era_flag != 0)) {
		PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_USAGE_TYPE,
					usage_type, ebufp);
	} else if (p_flistp &&
	    (fm_utils_is_subtype(e_pdp, PIN_OBJ_TYPE_EVENT_CYCLE_BASE)
	       || fm_utils_is_subtype(e_pdp, PIN_OBJ_TYPE_EVENT_PRODUCT_FEE_PURCHASE)
	       || fm_utils_is_subtype(e_pdp, PIN_OBJ_TYPE_EVENT_PRODUCT_FEE_CANCEL))) {

		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_PRICE_LIST_NAME,
			1, ebufp);
		PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_USAGE_TYPE, vp, ebufp);
	}
	
	if(head) {
		temp = head;
		strcpy(label,head->label);
		head = head->next; 
		while( head != NULL ) {
			max_size = sizeof(label) - strlen(label);
			pin_strlcat(label,";", max_size);
			pin_strlcat(label,head->label, max_size - 1); 
			head = head->next;            /* goto the next node in the list       */
		}
		head = temp;
	}
	pin_free(era_flag);
	freenode(head);

	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_PROFILE_LABEL_LIST,label, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Exit fm_rate_pol_proc_eras_set_era_name: ", *r_flistp );
	
}

static void
fm_rate_pol_proc_eras_get_cug_era(
	pin_flist_t	*p_flistp,
	char		*a_cug_value,
	int		*cug_flag,
	pin_errbuf_t	*ebufp)
{
	int32			elem_id = 0;
	int32			new_elem_id = 0;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		new_cookie = NULL;
	pin_flist_t		*prof_flistp = NULL;
	pin_flist_t		*shared_flistp = NULL;
	char			*prof_name = NULL;
	char			*b_cug_value = NULL;
	pin_flist_t		*era_flistp = NULL;


	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}

	/**********************************************************************
	* In case of normal cug the values are present in 
	* PIN_FLD_PROFILE_DATA_ARRAY 
	* but in case of profile sharing the values are present in 
	* PIN_FLD_PROFILE_DATA_ARRAY under PIN_FLD_SHARED_LIST
	************************************************************************/
	shared_flistp = PIN_FLIST_ELEM_GET(p_flistp,
		PIN_FLD_SHARED_LIST, 0, 1, ebufp);	
	if (shared_flistp) {
		p_flistp = shared_flistp;
	}

	while ((prof_flistp = PIN_FLIST_ELEM_GET_NEXT(p_flistp, PIN_FLD_PROFILE_DATA_ARRAY,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {
	
		prof_name = (char *) PIN_FLIST_FLD_GET (prof_flistp, 
					PIN_FLD_PROFILE_NAME, 0, ebufp);
		if((prof_name) && !strcmp(prof_name, CLOSED_USERGROUP)) {

			new_elem_id = 0;
			new_cookie =  NULL;

			while ((era_flistp = PIN_FLIST_ELEM_GET_NEXT(prof_flistp, PIN_FLD_DATA_ARRAY,
					&new_elem_id, 1, &new_cookie, ebufp)) != (pin_flist_t *)NULL) {
				
				b_cug_value = (char *) PIN_FLIST_FLD_GET(era_flistp,
						PIN_FLD_VALUE, 0, ebufp);
				if(a_cug_value && b_cug_value && !strcmp(a_cug_value, b_cug_value)) {
					/* Match found, not interested in other values */
					*cug_flag = 1;
					return;
				}
			}

		}
	}
}

static void
fm_rate_pol_proc_eras_get_b_account_eras(
	pcm_context_t	*ctxp,
	pin_flist_t	*e_flistp,
	char		*service_type,
	char		*b_num,
	pin_flist_t	**prof_flistp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t		*eras_inp_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*b_acct_flistp = NULL;
	pin_flist_t		*p_flistp = NULL;
	time_t			*evt_time = NULL;
	poid_t			*poid_pdp = NULL;
	int64			db_id = 0;
	poid_t			*s_pdp = NULL;
	void			*vp = NULL;
	poid_t			*e_pdp= NULL;
	


	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}

	b_acct_flistp = PIN_FLIST_CREATE(ebufp);

	poid_pdp = (poid_t *)PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db_id = PIN_POID_GET_DB(poid_pdp);
	s_pdp = PIN_POID_CREATE(db_id, service_type, -1, ebufp);

	PIN_FLIST_FLD_SET(b_acct_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(b_acct_flistp, PIN_FLD_LOGIN, b_num, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_act_find input flist: ", b_acct_flistp);

	PCM_OP(ctxp, PCM_OP_ACT_FIND, 0, b_acct_flistp, &r_flistp, ebufp);

	if(r_flistp != NULL) {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_act_find return flist: ", r_flistp);

		vp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		
		/* fetch Account B era only if account poid exists */	
		if(vp) {
			eras_inp_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
			e_pdp = (poid_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_POID, 0, ebufp);
			PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_POID, e_pdp, ebufp);

			vp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_POID, 0, ebufp);
			PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_SERVICE_OBJ, vp, ebufp);

			evt_time = (time_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_START_T, 0, ebufp);
			PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_START_T, evt_time, ebufp);

			evt_time = (time_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_END_T, 0, ebufp);
			PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_END_T, evt_time, ebufp);

			evt_time = (time_t *) PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp);
			if(evt_time && *evt_time) {
				PIN_FLIST_FLD_SET(eras_inp_flistp, PIN_FLD_EFFECTIVE_T, evt_time, ebufp);
			}

			/* calling PCM_OP_RATE_GET_ERAS opcde to to fetch Account B eras
			 * 3rd parameter is passed as 1 as to bypass the transaction catch
			 * operation for the eras
			 */

			PCM_OPREF(ctxp, PCM_OP_RATE_GET_ERAS, 1, eras_inp_flistp, &p_flistp, ebufp);
			*prof_flistp = p_flistp;

			PIN_FLIST_DESTROY_EX(&eras_inp_flistp, NULL);
		}
	}

	PIN_POID_DESTROY(s_pdp, NULL);
	PIN_FLIST_DESTROY_EX(&b_acct_flistp, NULL);
}


static void
fm_rate_pol_process_eras_set_profile_name (
	pcm_context_t		*ctxp,
	pin_flist_t		*profile_flistp,
	pin_flist_t		*e_flistp,
	char			*service_type,
	char			*matching_criteria, 
	int32			*era_flag,
	node			**head,
	pin_errbuf_t		*ebufp) 
{
	pin_flist_t		*profile_data_flistp = NULL;
	pin_flist_t		*shared_flistp = NULL;
	char			*prof_name = NULL;
	int32			elem_id = 0;
	pin_cookie_t		cookie = NULL;
	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Profile_flist..", profile_flistp);

	if(profile_flistp) {
		while ((profile_data_flistp  = PIN_FLIST_ELEM_GET_NEXT(profile_flistp, 
				PIN_FLD_PROFILE_DATA_ARRAY,  &elem_id, 1, &cookie, ebufp)) 
				!= (pin_flist_t *)NULL) {
		
			prof_name = (char *) PIN_FLIST_FLD_GET (profile_data_flistp, 
						PIN_FLD_PROFILE_NAME, 0, ebufp);
	
			fm_rate_pol_process_eras_populate_usagetype_label(ctxp,
							profile_data_flistp, e_flistp,
							service_type, matching_criteria,
							prof_name, era_flag, head, ebufp);
		}
		
		/************* For shared list profiles ********************/
		
 		shared_flistp = (pin_flist_t *) PIN_FLIST_ELEM_GET(profile_flistp,
 							PIN_FLD_SHARED_LIST, 0, 1, ebufp);
		
		elem_id = 0;
		cookie = NULL;
		if(shared_flistp) {
			while ((profile_data_flistp = PIN_FLIST_ELEM_GET_NEXT(shared_flistp, 
						PIN_FLD_PROFILE_DATA_ARRAY, &elem_id, 1, &cookie, ebufp)) 
						!= (pin_flist_t *)NULL) {
				prof_name = (char *) PIN_FLIST_FLD_GET (profile_data_flistp, 
							PIN_FLD_PROFILE_NAME, 0, ebufp);

				fm_rate_pol_process_eras_populate_usagetype_label(ctxp,
							profile_data_flistp, e_flistp,
							service_type, matching_criteria,
							prof_name, era_flag, head, ebufp);
			}
		}
	}
}

static void
fm_rate_pol_process_eras_populate_usagetype_label(
	pcm_context_t		*ctxp,
	pin_flist_t		*profile_data_flistp,
	pin_flist_t		*e_flistp,
	char			*service_type,
	char			*matching_criteria,   
	char			*profile_name,
	int32			*era_flag,
	node			**head,
	pin_errbuf_t		*ebufp)
 {

	pin_flist_t		*era_flistp = NULL;
	pin_flist_t		*extrating_flistp = NULL;
	node			*ptr = NULL;
	int32			elem_id = 0;
	pin_cookie_t		cookie = NULL;
	char			*era_value = NULL;
	char			*label = NULL;
	int32			ret_val = 0;
	
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Profile_data_flist..", profile_data_flistp);
	
	while ((era_flistp = PIN_FLIST_ELEM_GET_NEXT(profile_data_flistp, 
				PIN_FLD_DATA_ARRAY, &elem_id, 1, &cookie, ebufp)) 
				!= (pin_flist_t *)NULL) {
			
		era_value = PIN_FLIST_FLD_GET(era_flistp, PIN_FLD_VALUE, 0, ebufp );
		ret_val   = fm_rate_pol_proc_eras_value_match(ctxp,e_flistp,profile_name, 
				era_value, matching_criteria,service_type, era_flag, ebufp);

		if(ret_val) {
			break;
		}
	}
	
	if(ret_val) {
				
		/******************************************************************
		 * PIN_FLD_LABEL value needs to be taken from the PIN_FLD_EXTRATING
		 * substruct. 
		 *******************************************************************/

		extrating_flistp = PIN_FLIST_SUBSTR_GET(profile_data_flistp, 
							PIN_FLD_EXTRATING, 1, ebufp);
		if(extrating_flistp) {
			label = (char *) PIN_FLIST_FLD_GET(extrating_flistp, 
							PIN_FLD_LABEL, 0, ebufp);
			if(label) { 
	
				/*************************************************************
				 * Add the label value in the PIN_FLD_PROFILE_LABEL_LIST field 
				 * with the separator value which is coming as a input parameter 
				 * into this method. While adding the label value into the 
				 * PIN_FLD_PROFILE_LABEL_LIST duplication needs to be checked.
				 *************************************************************/
				ptr = initnode(label);
				insertnode(head, ptr); 
			}
		} 
	}

}
static int32 
fm_rate_pol_proc_eras_value_match(
	pcm_context_t		*ctxp,
	pin_flist_t		*e_flistp,
	char			*profile_name, 
	char			*era_value, 
	char			*matching_criteria,
	char			*svc,
	int32			*era_flag,
	pin_errbuf_t		*ebufp)
{

	int			cug_flag = 0;
	int			return_value = 0;
	pin_flist_t		*b_prof_flistp = NULL;
	
	
	if((profile_name) && strcmp(profile_name, FRIENDS_FAMILY) == 0) {
		return_value = prefix_and_suffix_match(era_value,matching_criteria);
		if(return_value && !(*era_flag & FF)) {
			*era_flag |= FF;
	   	}
	} else if((profile_name) && strcmp(profile_name, CLOSED_USERGROUP) == 0) {
		if(!(*era_flag & CUG)) {
			fm_rate_pol_proc_eras_get_b_account_eras(ctxp,
					e_flistp, svc, matching_criteria, &b_prof_flistp, ebufp);	
							
			if(b_prof_flistp) {
				fm_rate_pol_proc_eras_get_cug_era(b_prof_flistp,
							 era_value, &cug_flag, ebufp);

				if(cug_flag) {
					*era_flag |= CUG ;
				}
			}
		}
	}
	
	return return_value;
}

/**********************************************************************
 * this method initialises a node, allocates memory for the node, 
 * and returns a pointer to the new node. 
 **********************************************************************/
node * initnode( char *label )
{
   node *ptr;
   ptr = (node *) pin_malloc( sizeof(node) );
   if( ptr == NULL )                       
       return (node *) NULL;        
   else {                                  
       strcpy( ptr->label, label );        
       return ptr;                         
   }
}

/***************************************************************************
 * inserts a new node, pass it the address of the new node to be inserted, 
 * with details all filled in                                                              
 ***************************************************************************/

void 
insertnode( 
	node	**head,
	node	*newNodep )
{
   	
	node *temp = NULL;                
   
	if(!newNodep) {
		return;
	}
 
	if(*head == NULL) {                     
		*head = newNodep;                 
		(*head)->next = NULL;                
		return;                           
   	}

	temp = *head;                             
	
	while(temp != NULL) {
		if((temp->label) && (newNodep->label) && 
			strcmp(temp->label, newNodep->label) == 0 ) {
		   return;
		}
		if((temp->next) == NULL ) {
			temp->next = newNodep;
			newNodep->next = NULL;
			break;
		} else {
			temp = temp->next;                /* goto the next node in list */
		}
	}
		
	return;
}

int 
prefix_and_suffix_match(
		char*	source,
		char*	dest)
{
	int32	source_length =0;
        int32	prefix =0;
        int32	suffix =0;
        char	*s_ptr = source;
        char	*d_ptr = dest;
        char	*temp =  NULL;
        char	temp_str[256];


	if(!source || !dest) {
                return 0;
        }

        if((source) && (dest) && strcmp(source,dest) == 0) {
                return 1;
        }
	
	source_length =  strlen(source);
	
        if(source[0] != '*' && source[source_length - 1] != '*' ) {
                return 0;
        }

        /************ check prefix match *****************/
	if(source[0] == '*') {
		prefix = 1;
	}

	/************ check suffix match *****************/
	if(source[source_length - 1 ] == '*') {
		suffix = 1;
	}

	if(prefix && suffix) {
		while(*s_ptr ==  '*') {
			s_ptr ++;
		}
		temp = s_ptr;
		while(*s_ptr != '*') {
			s_ptr ++;
		}
		strncpy(temp_str, temp, s_ptr - temp);
		if(strstr(dest, temp_str) != NULL) {
			return 1;
		}
	} else if(prefix) {
		while(*s_ptr ==  '*') {
			s_ptr ++;
		}
		d_ptr += strlen(dest) - strlen(s_ptr);
		if((d_ptr) && (s_ptr) && !strcmp(d_ptr, s_ptr)) {
			return 1;
		}
	} else if (suffix) {
		while(*s_ptr !=  '*') {
			s_ptr ++;
		}
		if(!strncmp(dest, source, s_ptr - source))  {
			return 1;
		}
	}
	return 0;
}

void
freenode(
        node    *head)
{
	node *temp = NULL;

	if(!head) {
		return;
        }

	while(head) {
		temp = head->next;
		free(head);
		head = temp;
	}
}
