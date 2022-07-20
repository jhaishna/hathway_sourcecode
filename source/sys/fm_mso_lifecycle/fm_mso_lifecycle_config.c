/*
 * @(#)%Portal Version: fm_mso_lifecycle_config.c:ServerIDCVelocityInt:1:2006-Sep-14 11:34:40 %
 *
 * Copyright (c) 2001 - 2006 Oracle. All rights reserved.
 *
 * This material is the confidential property of Oracle Corporation
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_lifecycle_config.c:ServerIDCVelocityInt:1:2006-Sep-14 11:34:40 %";
#endif

#include <stdio.h>
#include <string.h>
#include <pcm.h>
#include <pinlog.h>
#include "ops/device.h"
#include "cm_fm.h"
#include "mso_ops_flds.h"
#define FILE_LOGNAME "fm_mso_lifecycle_config.c(1)"

#ifdef MSDOS
__declspec(dllexport) void * fm_mso_lifecycle_config_func();
#endif


/*******************************************************************
 *******************************************************************/
struct cm_fm_config fm_mso_lifecycle_config[] = {
	/* opcode as a u_int, function name (as a string) */
	{ MSO_OP_LIFECYCLE_DEVICE,	"op_mso_lifecycle_device" },
	//{ MSO_OP_DEVICE_BB_ASSOCIATE,		"op_mso_device_bb_associate" },
	{ 0,	(char *)0 }
};

#ifdef MSDOS
void *
fm_mso_lifecycle_config_func()
{
  return ((void *) (fm_mso_lifecycle_config));
}
#endif

