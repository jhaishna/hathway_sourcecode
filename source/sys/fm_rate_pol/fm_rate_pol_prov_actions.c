/*******************************************************************
 *
 * @(#)%Portal Version: fm_rate_pol_prov_actions.c:IDCmod7.3PatchInt:1:2007-Mar-26 14:15:36 %
 *
 *      Copyright (c) 2014 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its subsidiaries or licensors and may be used, reproduced, stored
 *      or transmitted only in accordance with a valid Oracle license or
 *      sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_rate_pol_prov_actions.c:IDCmod7.3PatchInt:1:2007-Mar-26 14:15:36 %";
#endif

/*******************************************************************
 * Contains the utility function used by FM_RATE_POL_* opcodes.
 *******************************************************************/

#include <stdio.h>
#include <string.h>

#include "pcm.h"
#include "ops/rate.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_act.h"
#include "ops/cust.h"
#include "pin_rate.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "mso_ops_flds.h"
#include "mso_prov.h"
#include "mso_glid.h"
#include "ops/bal.h"
#include "mso_rate.h"

EXPORT void raise_notification();

EXPORT void perform_provisioning_action();


void raise_notification() {

}


void perform_provisioning_action() {

}