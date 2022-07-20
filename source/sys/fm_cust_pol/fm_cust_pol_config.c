/*******************************************************************
 *
 *
* Copyright (c) 2007, 2011, Oracle and/or its affiliates. All rights reserved. 
 * 
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_custom_config.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 04:28:59 nishahan Exp $";
#endif

#include <stdio.h>			/* for FILE * in pcm.h */
#include "ops/cust.h"
#include "pcm.h"
#include "cm_fm.h"

PIN_EXPORT void * fm_cust_pol_config_func();

/*******************************************************************
 *******************************************************************/

struct cm_fm_config fm_cust_pol_custom_config[] = {
	/* opcode as a u_int, function name (as a string) */

        /*****************************************************************************
         * If you want to customize any of the op-codes commented below, you need to *
         * uncomment it.
         *****************************************************************************/

	{ PCM_OP_CUST_POL_PRE_COMMIT,		"op_cust_pol_pre_commit" },
	{ PCM_OP_CUST_POL_POST_COMMIT,		"op_cust_pol_post_commit" },
	{ PCM_OP_CUST_POL_GET_CONFIG,		"op_cust_pol_get_config" },
	{ PCM_OP_CUST_POL_GET_INTRO_MSG,	"op_cust_pol_get_intro_msg" },
	{ PCM_OP_CUST_POL_GET_PLANS,		"op_cust_pol_get_plans" },
	{ PCM_OP_CUST_POL_GET_DEALS,		"op_cust_pol_get_deals" },
	{ PCM_OP_CUST_POL_GET_PRODUCTS,		"op_cust_pol_get_products" },
	{ PCM_OP_CUST_POL_GET_POPLIST,		"op_cust_pol_get_poplist" },
	{ PCM_OP_CUST_POL_VALID_ACCTINFO,	"op_cust_pol_valid_acctinfo" },
	{ PCM_OP_CUST_POL_VALID_BILLINFO,	"op_cust_pol_valid_billinfo" },
	{ PCM_OP_CUST_POL_VALID_NAMEINFO,	"op_cust_pol_valid_nameinfo" },
	{ PCM_OP_CUST_POL_VALID_AACINFO,	"op_cust_pol_valid_aacinfo" },
	{ PCM_OP_CUST_POL_VALID_INHERITED,	"op_cust_pol_valid_inherited" },
	{ PCM_OP_CUST_POL_VALID_PASSWD,		"op_cust_pol_valid_passwd" },
	{ PCM_OP_CUST_POL_VALID_LOCALE,		"op_cust_pol_valid_locale" },
	{ PCM_OP_CUST_POL_VALID_LOGIN,		"op_cust_pol_valid_login",CM_FM_OP_OVERRIDABLE },
	{ PCM_OP_CUST_POL_VALID_LIMIT,		"op_cust_pol_valid_limit" },
	{ PCM_OP_CUST_POL_VALID_STATUS,		"op_cust_pol_valid_status" },
	{ PCM_OP_CUST_POL_PREP_ACCTINFO,	"op_cust_pol_prep_acctinfo" },
	{ PCM_OP_CUST_POL_PREP_BILLINFO,	"op_cust_pol_prep_billinfo", CM_FM_OP_OVERRIDABLE },
	{ PCM_OP_CUST_POL_PREP_NAMEINFO,	"op_cust_pol_prep_nameinfo" },
	{ PCM_OP_CUST_POL_PREP_AACINFO,		"op_cust_pol_prep_aacinfo" },
	{ PCM_OP_CUST_POL_PREP_INHERITED,	"op_cust_pol_prep_inherited" },
	{ PCM_OP_CUST_POL_PREP_PASSWD,		"op_cust_pol_prep_passwd" },
	{ PCM_OP_CUST_POL_PREP_LOGIN,		"op_cust_pol_prep_login" },
	{ PCM_OP_CUST_POL_PREP_LOCALE,		"op_cust_pol_prep_locale" },
	{ PCM_OP_CUST_POL_PREP_LIMIT,		"op_cust_pol_prep_limit" },
	{ PCM_OP_CUST_POL_PREP_STATUS,		"op_cust_pol_prep_status" },
	{ PCM_OP_CUST_POL_COMPARE_PASSWD,	"op_cust_pol_compare_passwd" },
	{ PCM_OP_CUST_POL_ENCRYPT_PASSWD,	"op_cust_pol_encrypt_passwd" },
	{ PCM_OP_CUST_POL_DECRYPT_PASSWD,	"op_cust_pol_decrypt_passwd" },
	{ PCM_OP_CUST_POL_EXPIRATION_PASSWD,	"op_cust_pol_expiration_passwd" }, 
	{ PCM_OP_CUST_POL_PREP_PROFILE,		"op_cust_pol_prep_profile" },
	{ PCM_OP_CUST_POL_VALID_PROFILE,	"op_cust_pol_valid_profile" },
	{ PCM_OP_CUST_POL_PREP_PAYINFO,		"op_cust_pol_prep_payinfo" },
	{ PCM_OP_CUST_POL_VALID_PAYINFO,	"op_cust_pol_valid_payinfo" },
	{ PCM_OP_CUST_POL_MODIFY_SERVICE,	"op_cust_pol_modify_service"},
	{ PCM_OP_CUST_POL_CANONICALIZE, 	"op_cust_pol_canonicalize"},
	{ PCM_OP_CUST_POL_VALID_TAXINFO, 	"op_cust_pol_valid_taxinfo"},
	{ PCM_OP_CUST_POL_GET_DB_NO, 		"op_cust_pol_get_db_no"},
	{ PCM_OP_CUST_POL_READ_PLAN, 		"op_cust_pol_read_plan"}, 
	{ PCM_OP_CUST_POL_TAX_INIT, 		"op_cust_pol_tax_init", CM_FM_OP_OVERRIDABLE },
	{ PCM_OP_CUST_POL_TAX_CALC, 		"op_cust_pol_tax_calc", CM_FM_OP_OVERRIDABLE },
	{ PCM_OP_CUST_POL_GET_DB_LIST,		"op_cust_pol_get_db_list"},
	{ PCM_OP_CUST_POL_SET_BRANDINFO,	"op_cust_pol_set_brandinfo"},
	{ PCM_OP_CUST_POL_POST_MODIFY_CUSTOMER,	"op_cust_pol_post_modify_customer"},
	{ PCM_OP_CUST_POL_TRANSITION_DEALS,     "op_cust_pol_transition_deals"},
	{ PCM_OP_CUST_POL_TRANSITION_PLANS,     "op_cust_pol_transition_plans"},
	{ PCM_OP_CUST_POL_GET_SUBSCRIBED_PLANS, "op_cust_pol_get_subscribed_plans"},
	{ PCM_OP_CUST_POL_VALID_TOPUP,		"op_cust_pol_valid_topup"},
	{ PCM_OP_CUST_POL_PREP_TOPUP,		"op_cust_pol_prep_topup"},
/*        { PCM_OP_CUST_POL_PRE_DELETE_PAYINFO,	"op_cust_pol_pre_delete_payinfo"},*/
	{ 0,	(char *)0 }
};

void *
fm_cust_pol_custom_config_func()
{
  return ((void *) (fm_cust_pol_custom_config));
}

