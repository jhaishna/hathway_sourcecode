/*
* Copyright (c) 2001, 2014, Oracle and/or its affiliates. All rights reserved.
 *
 * This material is the confidential property of Oracle Corporation
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static const char Sccs_id[] = "@(#)$Id: fm_device_pol_custom_config.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 03:59:02 nishahan Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <pcm.h>
#include <pinlog.h>
#include "ops/device.h"
#include "cm_fm.h"

#define FILE_LOGNAME "fm_device_pol_config.c(1)"

#ifdef MSDOS
__declspec(dllexport) void * fm_device_pol_custom_config_func();
#endif


/*******************************************************************
 *******************************************************************/
struct cm_fm_config fm_device_pol_custom_config[] = {
	/* opcode as a u_int, function name (as a string) */
	{ PCM_OP_DEVICE_POL_SET_STATE,		"op_device_pol_set_state" },
	{ PCM_OP_DEVICE_POL_SET_ATTR,		"op_device_pol_set_attr" },
	{ PCM_OP_DEVICE_POL_DELETE,		"op_device_pol_delete" },
	{ PCM_OP_DEVICE_POL_REPLACE,		"op_device_pol_replace" },
	{ PCM_OP_DEVICE_POL_SET_BRAND,		"op_device_pol_set_brand" } ,
	{ PCM_OP_DEVICE_POL_CREATE,		"op_device_pol_create"},
	{ PCM_OP_DEVICE_POL_ASSOCIATE,		"op_device_pol_associate"},
	{ 0,	(char *)0 }
};

#ifdef MSDOS
void *
fm_device_pol_custom_config_func()
{
  return ((void *) (fm_device_pol_custom_config));
}
#endif

