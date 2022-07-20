/*******************************************************************
 * File:	fm_mso_cust_get_bouquet_ids.c
 * Opcode:	MSO_OP_CUST_GET_BOUQUET_IDS 
 * Created:	19-DEC-2018	
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * This opcode is to retrieve bouquet ids for the give city (area wise if applicable)
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_get_bouquet_ids.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"

/*******************************************************************
 * MSO_OP_CUST_GET_BOUQUET_IDS 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_get_bouquet_ids(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_bouquet_ids(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);


static void
fm_mso_get_city_bat_id_details(
        pcm_context_t    *ctxp,
        char           *city_bat_id,
        pin_flist_t      **ret_flistp,
        pin_errbuf_t     *ebufp);

static void
fm_mso_get_area_bat_id_details(
        pcm_context_t    *ctxp,
        char             *city_bat_id,
        pin_flist_t      **ret_flistp,
        pin_errbuf_t     *ebufp);

static void 
fm_mso_remove_duplicates(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

/*******************************************************************
 * MSO_OP_CUST_GET_BOUQUET_IDS  
 *******************************************************************/
void 
op_mso_cust_get_bouquet_ids(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_GET_BOUQUET_IDS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_get_bouquet_ids error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_get_bouquet_ids input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_get_bouquet_ids(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_bouquet_ids error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_get_bouquet_ids output flist", *r_flistpp);
	}
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_get_bouquet_ids(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*city_bouquet_flistp = NULL;
	pin_flist_t		*area_bouquet_flistp = NULL;
	char			*cityName = NULL;
	int32			count = 0;
	int32			set_get_bouquet_ids_failure = 1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_bouquet_ids input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in calling fm_mso_cust_get_bouquet_ids", ebufp);
	}

	cityName = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY, 0, ebufp);

	if (cityName != NULL && strlen(cityName)>0) {

		fm_mso_get_city_bat_id_details(ctxp, cityName, &city_bouquet_flistp, ebufp);
		count = PIN_FLIST_ELEM_COUNT(city_bouquet_flistp, PIN_FLD_RESULTS, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_get_bouquet_ids error",ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_get_bouquet_ids_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84001", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Bouquet IDs (CITY) Search Error", ebufp);
			goto cleanup;
		}

		if (count == 0) {
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                	ret_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_get_bouquet_ids_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84000", ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No Bouquet IDs found", ebufp);
			goto cleanup;
		}
		else {
			ret_flistp = PIN_FLIST_COPY(city_bouquet_flistp, ebufp);
		}

		fm_mso_get_area_bat_id_details(ctxp, cityName, &area_bouquet_flistp, ebufp);
		count = PIN_FLIST_ELEM_COUNT(area_bouquet_flistp, PIN_FLD_RESULTS, ebufp);

                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_get_bouquet_ids error",ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_get_bouquet_ids_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84001", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Bouquet IDs (AREA) Search Error", ebufp);
			goto cleanup;
                }
                if (count == 0) {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Area wise Bouquet IDs found");
                }
		else {
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
			ret_flistp = PIN_FLIST_COPY(area_bouquet_flistp, ebufp);
		}
	}
	else{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_get_bouquet_ids_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "94002", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "City Name is null", ebufp);
		goto cleanup;	
	}

	fm_mso_remove_duplicates(ctxp, ret_flistp, r_flistpp, ebufp);;;;
	
cleanup:
	//*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&city_bouquet_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&area_bouquet_flistp, NULL);

	return;
}

static void
fm_mso_get_area_bat_id_details(
        pcm_context_t    *ctxp,
	char 		 *city_bat_id,
        pin_flist_t      **ret_flistp,
        pin_errbuf_t     *ebufp)
{
        pin_flist_t     *srch_iflistp=NULL;
        pin_flist_t     *srch_oflistp=NULL;
        pin_flist_t     *arg_flistp=NULL;
        pin_flist_t     *res_flistp=NULL;

		
        char    *template="select X from /mso_cfg_bouquet_area_map 1 , /mso_cfg_bouquet_city_map 2 where 2.F1 = V1 and instr(mso_cfg_bouquet_area_map_t.area_code, mso_cfg_bouquet_city_map_t.city_code) != 0 order by mso_cfg_bouquet_area_map_t.bouquet_id ";
        int64   db = 1;
        int32   srch_flag = 256;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in before searching prov tag", ebufp);
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_area_bat_id_details ");

        srch_iflistp= PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CITY, city_bat_id, ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_BOUQUET_ID, NULL, ebufp);
        
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_area_bat_id_details ");

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_area_bat_id_details search input flist is : ", srch_iflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_area_bat_id_details search output flist is : ", srch_oflistp);

       *ret_flistp = PIN_FLIST_COPY(srch_oflistp,ebufp);

        PIN_FLIST_DESTROY_EX(&srch_iflistp,NULL);
        PIN_FLIST_DESTROY_EX(&srch_oflistp,NULL);

        return;
}


static void
fm_mso_get_city_bat_id_details(
        pcm_context_t    *ctxp,
        char           *city_bat_id,
        pin_flist_t      **ret_flistp,
        pin_errbuf_t     *ebufp)
{
        pin_flist_t     *srch_iflistp=NULL;
        pin_flist_t     *srch_oflistp=NULL;
        pin_flist_t     *arg_flistp=NULL;
        pin_flist_t     *res_flistp=NULL;

        char    *template="select X from /mso_cfg_bouquet_city_map where F1 = V1 ";
        int64   db = 1;
        int32   srch_flag = 256;
        //char                    *city_bat_id = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in before searching prov tag", ebufp);
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_city_bat_id_details ");

        srch_iflistp= PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CITY, city_bat_id, ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_BOUQUET_ID, NULL, ebufp);
        
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_city_bat_id_details ");

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_city_bat_id_details search input flist is : ", srch_iflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_city_bat_id_details search output flist is : ", srch_oflistp);

       *ret_flistp = PIN_FLIST_COPY(srch_oflistp, ebufp);

        PIN_FLIST_DESTROY_EX(&srch_iflistp,NULL);
        PIN_FLIST_DESTROY_EX(&srch_oflistp,NULL);
        return;
}

static void
fm_mso_remove_duplicates(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp) {

	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;

	pin_cookie_t	cookie = NULL;
	int32 	elem_id = 0;
	int32 	counter = 0;
	char 	*bouquet_id = NULL;
	char 	prev_bouquet_id[128];

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in before searching prov tag", ebufp);
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_remove_duplicates input flist ", i_flistp);

	result_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp);
	bouquet_id = (char*)PIN_FLIST_FLD_GET(result_flistp, MSO_FLD_BOUQUET_ID, 0, ebufp);
	strcpy(prev_bouquet_id, bouquet_id);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_DATA_ARRAY, counter, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_BOUQUET_ID, prev_bouquet_id, ebufp);
	
	while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS, 
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {
		bouquet_id = (char*)PIN_FLIST_FLD_GET(result_flistp, MSO_FLD_BOUQUET_ID, 0, ebufp);
		if (strcmp(bouquet_id, prev_bouquet_id) == 0) {
			continue;
		}
		else {
			tmp_flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_DATA_ARRAY, ++counter, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
			strcpy(prev_bouquet_id, bouquet_id);
		}
		
	}
	*r_flistpp = r_flistp;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_remove_duplicates output flist ", *r_flistpp);
}
