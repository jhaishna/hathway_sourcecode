/**********************************************************************
* Author        : Vilva Sabarikanth
* Purpose       : 
*		  
* Date          : 
**********************************************************************/
/*********************************************************************
New Custom opcode MSO_OP_PYMT_AUTOPAY . 
This opcode is a wrapper to build input flist for MSO_OP_PYMT_COLLECT.
**********************************************************************/



#include <stdio.h>
#include <string.h>
#include <time.h>

#include "pcm.h"
#include "ops/pymt.h"
#include "pin_rate.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_ar.h"
#include "ops/bill.h"
#include "mso_ops_flds.h"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/

EXPORT_OP void
mso_pymt_autopay(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void
mso_pymt_autopay_collect(
        pcm_context_t		*ctxp,
	int                     flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
update_mso_lco_autopay_master(
        pcm_context_t           *ctxp,
        int                     flags,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * Main routine
 *******************************************************************/
void
mso_pymt_autopay(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	* Null out results until we have some.
	***********************************************************/
	*o_flistpp = NULL;

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_AUTOPAY) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_pymt_autopay opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_pymt_autopay input flist", i_flistp);

	mso_pymt_autopay_collect(ctxp, flags, i_flistp, &r_flistp, ebufp);
	
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*o_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		    "mso_pymt_autopay error", ebufp);
        } else {
		update_mso_lco_autopay_master(ctxp, flags, i_flistp, ebufp);
		
		*o_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		    "mso_pymt_autopay return flist", *o_flistpp);

	}
        return;
}

/***********************************************************
 * prepare input flist for MSO_OP_PYMT_COLLECT.
 ***********************************************************/
static void
mso_pymt_autopay_collect(
        pcm_context_t		*ctxp,
	int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
        pin_flist_t             *pymt_in_flistp = NULL;
        pin_flist_t             *elem_flistp = NULL;
        pin_flist_t             *substr_flistp = NULL;
	pin_flist_t             *inherited_flistp = NULL;
	pin_flist_t             *cash_flistp = NULL;
        pin_flist_t             *pymt_ret_flistp = NULL;
	void			*vp = NULL;
	int32			i_val = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/****************************************************************************
	 * Input flist for MSO_OP_PYMT_COLLECT.
	 * 0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 426849 0
	 * 0 PIN_FLD_PROGRAM_NAME    STR [0] "MSO_AUTOPAY"
	 * 0 MSO_FLD_PYMT_CHANNEL   ENUM [0] 0 
	 * 0 MSO_FLD_SERVICE_TYPE ENUM [0] 1
	 * 0 PIN_FLD_USERID        POID [0] 0.0.0.1 /account 353527 0
	 * 0 PIN_FLD_TYPE           ENUM [0] 0
	 * 0 PIN_FLD_START_T      TSTAMP [0] (1432237038)
	 * 0 PIN_FLD_END_T        TSTAMP [0] (1432237038)
	 * 0 PIN_FLD_DESCR         STR [0] "Cash Payment"
	 * 0 PIN_FLD_CHARGES       ARRAY [0] allocated 20, used 5
	 * 1     PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 426849 0
	 * 1     PIN_FLD_COMMAND        ENUM [0] 0
	 * 1     PIN_FLD_PAY_TYPE       ENUM [0] 10019
	 * 1     PIN_FLD_AMOUNT       DECIMAL [0] 200.00
	 * 1     PIN_FLD_PAYMENT            SUBSTRUCT [0] allocated 20, used 2
	 * 2       PIN_FLD_INHERITED_INFO   SUBSTRUCT [0] allocated 20, used 2
	 * 3           PIN_FLD_CASH_INFO              ARRAY [0] allocated 20, used 5
	 * 4              PIN_FLD_RECEIPT_NO       STR [0]  "R-09087654"
	 * 4              PIN_FLD_EFFECTIVE_T    TSTAMP [0] (1432237038)
	*****************************************************************************/

	pymt_in_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET (i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET (pymt_in_flistp, PIN_FLD_POID, vp, ebufp);

	PIN_FLIST_FLD_SET (pymt_in_flistp, 
		PIN_FLD_PROGRAM_NAME, (void *) "MSO_AUTOPAY", ebufp);
	PIN_FLIST_FLD_SET (pymt_in_flistp, MSO_FLD_PYMT_CHANNEL, (void *) &i_val, ebufp);
	i_val = 0; // Changed to 0, as 1 is used by Broadband
	PIN_FLIST_FLD_SET (pymt_in_flistp, MSO_FLD_SERVICE_TYPE, (void *) &i_val, ebufp);
	PIN_FLIST_FLD_SET (pymt_in_flistp, PIN_FLD_USERID, vp, ebufp);
	i_val = 0;
	PIN_FLIST_FLD_SET (pymt_in_flistp, PIN_FLD_TYPE, (void *) &i_val, ebufp);
	PIN_FLIST_FLD_SET (pymt_in_flistp, PIN_FLD_START_T, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET (pymt_in_flistp, PIN_FLD_END_T, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET (pymt_in_flistp, PIN_FLD_DESCR, (void *) "Cash Payment" , ebufp);
	elem_flistp = PIN_FLIST_ELEM_ADD(pymt_in_flistp, PIN_FLD_CHARGES, 0, ebufp);
	PIN_FLIST_FLD_SET (elem_flistp, PIN_FLD_ACCOUNT_OBJ, vp , ebufp);
	i_val = 0;
	PIN_FLIST_FLD_SET (elem_flistp, PIN_FLD_COMMAND, (void *) &i_val, ebufp);
	i_val = MSO_PAY_TYPE_SP_AUTOPAY;
	PIN_FLIST_FLD_SET (elem_flistp, PIN_FLD_PAY_TYPE, (void *) &i_val, ebufp);
	vp = PIN_FLIST_FLD_GET (i_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);
	PIN_FLIST_FLD_SET (elem_flistp, PIN_FLD_AMOUNT, (void *) vp, ebufp);
	substr_flistp = PIN_FLIST_SUBSTR_ADD(elem_flistp, PIN_FLD_PAYMENT, ebufp);
	inherited_flistp = PIN_FLIST_SUBSTR_ADD(substr_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	cash_flistp = PIN_FLIST_ELEM_ADD(inherited_flistp, PIN_FLD_CASH_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET (cash_flistp, PIN_FLD_RECEIPT_NO, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET (cash_flistp, PIN_FLD_EFFECTIVE_T, (void *)NULL, ebufp);

	PCM_OP(ctxp, MSO_OP_PYMT_COLLECT, 0, pymt_in_flistp, &pymt_ret_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pymt_ret_flistp output", pymt_ret_flistp);
	PIN_FLIST_DESTROY_EX(&pymt_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pymt_ret_flistp error", ebufp);
		PIN_FLIST_DESTROY_EX(&pymt_ret_flistp, NULL);
		*ret_flistpp = NULL;
	}else{
		*ret_flistpp = pymt_ret_flistp;
	}

	return;
}

/***********************************************************
 * prepare input flist for MSO_OP_PYMT_COLLECT.
 ***********************************************************/
static void
update_mso_lco_autopay_master(
        pcm_context_t           *ctxp,
        int			flags,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_iflistp = NULL;
	pin_flist_t             *srch_oflistp = NULL;
	pin_flist_t             *arg_flistp = NULL;
	pin_flist_t             *results_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *inc_iflistp = NULL;
	pin_flist_t             *inc_rflistp = NULL;
	poid_t			*search_pdp = NULL;
	int64			db_no = 0;
	char			*vp = NULL;
	char            	*templatep = " select X from /mso_lco_autopay_master where F1 = V1 and F2 = V2 " ;
        int32           iflags = 256;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET (i_flistp, PIN_FLD_POID, 0, ebufp);
	if(vp){
		db_no = PIN_POID_GET_DB ((poid_t*)vp);
	}

	search_pdp = PIN_POID_CREATE(db_no, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID,(void *)search_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &iflags, ebufp);
	PIN_FLIST_FLD_SET( srch_iflistp, PIN_FLD_TEMPLATE, (void *)templatep, ebufp);

	vp = PIN_FLIST_FLD_GET (i_flistp, MSO_FLD_ACCOUNT_OWNER, 0, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_NO, vp, ebufp);

	vp = PIN_FLIST_FLD_GET (i_flistp, PIN_FLD_REFERENCE_ID, 0, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REFERENCE_ID, vp, ebufp);

	results_flistp=PIN_FLIST_ELEM_ADD( srch_iflistp, PIN_FLD_RESULTS, PCM_RECID_ALL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Search Input Flist ",srch_iflistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Search error", ebufp);

			PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Search Output Flist ",srch_oflistp);

	res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp);

	inc_iflistp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET (res_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(inc_iflistp, PIN_FLD_POID, vp, ebufp);
	vp = PIN_FLIST_FLD_GET (i_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);
	PIN_FLIST_FLD_SET(inc_iflistp, MSO_FLD_AMOUNT_TOTAL, vp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Increment Input Flist ",inc_iflistp);
	
	PCM_OP(ctxp, PCM_OP_INC_FLDS, 0, inc_iflistp, &inc_rflistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Increment Output Flist ",inc_rflistp);

	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&inc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&inc_rflistp, NULL);

	return;
	

}
