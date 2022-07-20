/*
 * @(#)%Portal Version: fm_mso_device_config.c:ServerIDCVelocityInt:1:2006-Sep-14 11:34:40 %
 *
 * Copyright (c) 2001 - 2006 Oracle. All rights reserved.
 *
 * This material is the confidential property of Oracle Corporation
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_device_config.c:ServerIDCVelocityInt:1:2006-Sep-14 11:34:40 %";
#endif

#include <stdio.h>
#include <string.h>
#include <pcm.h>
#include <pinlog.h>
#include "ops/device.h"
#include "cm_fm.h"
#include "mso_ops_flds.h"
#define FILE_LOGNAME "fm_mso_device_config.c(1)"

#ifdef MSDOS
__declspec(dllexport) void * fm_mso_device_config_func();
#endif


/*******************************************************************
 *******************************************************************/
struct cm_fm_config fm_mso_device_config[] = {
	/* opcode as a u_int, function name (as a string) */
	{ MSO_OP_DEVICE_CATV_ASSOCIATE,		"op_mso_device_catv_associate" },
	{ MSO_OP_DEVICE_GET_BUFFER,		"op_mso_utils_get_buffer" },
	{ MSO_OP_DEVICE_CATV_PREACTIVATION,	"op_mso_device_catv_preactivation" },
	{ MSO_OP_UTILS_BLOB_TO_FLIST,	"op_mso_utils_blob_to_flist" },
	{ MSO_OP_DEVICE_CATV_SWAP,	"op_mso_device_catv_swap" },
	{ MSO_OP_DEVICE_BLACKLIST_VC,	"op_mso_device_vc_blacklist" },
	{ MSO_OP_DEVICE_BB_ASSOCIATE,		"op_mso_device_bb_associate" },
	{ MSO_OP_DEVICE_CATV_DISASSOCIATE,      "op_mso_device_catv_dis_associate" },
	{ MSO_OP_GRV_UPLOAD,      "op_mso_grv_upload" },
	{ MSO_OP_DEVICE_SET_ATTR,      "op_mso_device_set_attr" },
	{ MSO_OP_DEVICE_SET_STATE,      "op_mso_device_set_state" },
	{ MSO_OP_DEVICE_CHILD_REGISTRATION,      "op_mso_device_child_registration" },
	{ MSO_OP_DEVICE_FAULTY_TO_REPAIR,	"op_mso_device_faulty_to_repair" },
	{ MSO_OP_DEVICE_CREATE,      "op_mso_device_create" },
	{ MSO_OP_DEVICE_PAIR,      "op_mso_device_pair" },
	/*{ MSO_OP_DEVICE_GET_CHALLAN,      "op_mso_device_get_challan" },
	{ MSO_OP_DEVICE_VIEW_CHALLAN,      "op_mso_device_view_challan" },
	{ MSO_OP_DEVICE_GENERATE_CHALLAN,      "op_mso_device_generate_challan" },*/
	{ 0,	(char *)0 }
};

#ifdef MSDOS
void *
fm_mso_device_config_func()
{
  return ((void *) (fm_mso_device_config));
}
#endif

