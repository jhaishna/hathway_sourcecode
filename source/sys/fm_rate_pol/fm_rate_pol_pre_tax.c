/*******************************************************************
 *
* Copyright (c) 2000, 2016, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its subsidiaries or licensors and may be used, reproduced, stored
 *      or transmitted only in accordance with a valid Oracle license or
 *      sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_id[] = "@(#)$Id: fm_rate_pol_pre_tax.c /cgbubrm_7.5.0.portalbase/2 2016/07/20 14:47:53 vivilin Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_RATE_POL_PRE_TAX operation. 
 *
 * Used to provide a chance to modify/update any tax data 
 * prior to sending it to the DM for tax calculation.
 *
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/rate.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

#define FILE_SOURCE_ID  "fm_rate_pol_pre_tax.c"

/*******************************************************************
*  PLEASE UNCOMMENT THE FOLLOWING HEADER FILE 
*  DECLARATIONS AND  #define CONSTANTS  TO DO APPROPRIATE
* CUSTOMIZATION  FOR VERTEX  Q SERIES.
********************************************************************/

/********************************************************************
#include "stda.h"
#include "ctqa.h"
********************************************************************/
/********************************************************************
#define ADD_CUSTOM_OUT_FLD_VERTEX_QSERIES 1
********************************************************************/
/********************************************************************
#define USE_VERTEX_Q_SERIES 1 
*********************************************************************/

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_rate_pol_pre_tax(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_rate_pol_pre_tax(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * Main routine for the PCM_OP_RATE_POL_PRE_TAX operation.
 *******************************************************************/
void
op_rate_pol_pre_tax(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != PCM_OP_RATE_POL_PRE_TAX) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_pre_tax opcode error", ebufp);
		return;
	}

	/*
	 * Debug: What we got.
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_rate_pol_pre_tax input flist", i_flistp);

	/*
	 * Do the actual op in a sub.
	 */
	fm_rate_pol_pre_tax(ctxp, i_flistp, o_flistpp, ebufp);

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_pre_tax error", ebufp);
	} else {
		/*
		 * Debug: What we're sending back.
		 */
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_rate_pol_pre_tax return flist", *o_flistpp);
	}

	return;
}

/*******************************************************************
 * fm_rate_pol_pre_tax():
 *
 *    Use this policy to modify/update any tax information in the
 *    input flist for the tax calculation.  
 *
 *******************************************************************/
static void
fm_rate_pol_pre_tax(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t	*ret_flistp = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Prepare the return flist
	 */
	ret_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

#ifdef OVERRIDE_BY_ZIP
	/* For Pkg=CTQ and LocMode=0, let's override by Zip */ 
	{
	int32	loc = -1;	/* location mode */
	int32	pkg = -1;	/* package type */
	int32	ind = 1;	/* type of call = North America */
	int32	elemid = 0;
	char 	buf[1024];
	char	*bogus_num = "408572";
	void	*ship_t = NULL;
	void	*ship_f = NULL;
	void 	*order_a = NULL;
	void	*order_o = NULL;
	pin_cookie_t cookie = NULL;
	pin_flist_t *t_flistp = NULL;

	while ((t_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistp, PIN_FLD_TAXES, 
		&elemid, 1, &cookie, ebufp)) != (pin_flist_t*)NULL) {

		/* get the package type */
		vp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_TAXPKG_TYPE, 
			1, ebufp);
		if (vp != (void*) NULL) {
			pkg = *(int*)vp;
		}

		/* get the location access mode */
		vp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_LOCATION_MODE, 
			1, ebufp);
		if (vp != (void*) NULL) {
			loc = *(int*)vp;
		}

		if ((pkg != PIN_RATE_TAXPKG_COMMTAX) || 
			(loc != PIN_RATE_LOC_ADDRESS)) {

			continue;
		}

		/* For Pkg=CTQ and LocMode=0, let's override by Zip */ 

		order_o = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_ORDER_ORIGIN, 
			1, ebufp);
		order_a = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_ORDER_ACCEPT, 
			1, ebufp);
		ship_t = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_SHIP_TO, 
			1, ebufp);
		ship_f = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_SHIP_FROM, 
			1, ebufp);

		/* modify the addresses to include telco information */
		if (ship_t != (void*)NULL) {
			pin_snprintf(buf, sizeof(buf), "%s; [%s,%d,%d]", (char*)ship_t,
				bogus_num, loc, ind);
			PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_SHIP_TO,
				(void*)buf, ebufp);
		}
		if (ship_f != (void*)NULL) {
			pin_snprintf(buf, sizeof(buf), "%s; [%s,%d,%d]", (char*)ship_f,
				bogus_num, loc, ind);
			PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_SHIP_FROM,
				(void*)buf, ebufp);
		}
		if (order_o != (void*)NULL) {
			pin_snprintf(buf, sizeof(buf), "%s; [%s,%d,%d]", (char*)order_o,
				bogus_num, loc, ind);
			PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_ORDER_ORIGIN,
				(void*)buf, ebufp);
		}
		if (order_a != (void*)NULL) {
			pin_snprintf(buf, sizeof(buf), "%s; [%s,%d,%d]", (char*)order_a,
				bogus_num, loc, ind);
			PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_ORDER_ACCEPT,
				(void*)buf, ebufp);
		}

		/* set the international indicator */
		PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_INTERNATIONAL_IND, 
			&ind, ebufp);

	}
	/* end of override sample code */
	}
#endif

#ifdef USE_VERTEX_Q_SERIES
	/***************************************************************** 
	* FOLLOWING MANDATORY FIELDS SHOULD BE FILLED WITH CORRECT VALUES . 
  	* YOU CAN CUSTOMIZE THE THE MADATORY AND OPTIONAL FIELDS.
	* HERE IS THE LIST OF MAPPING FIELDS.
	* VERTEX DATATYPE  => MAPPING FIELD		=>PORTAL DATATYPE
	* 
	* eCtqString 	=>PIN_FLD_VERTEX_CTQ_STR	=>PIN_FLDT_STR
	* eCtqInt	=>PIN_FLD_VERTEX_CTQ_INT       	=>PIN_FLDT_INT
	* eCtqFloat	=>PIN_FLD_VERTEX_CTQ_FLOAT     	=>PIN_FLDT_DECIMAL
	* eCtqDouble	=>PIN_FLD_VERTEX_CTQ_DOUBLE    	=>PIN_FLDT_DECIMAL
	* eCtqLong	=>PIN_FLD_VERTEX_CTQ_LONG      	=>PIN_FLDT_DECIMAL
	* eCtqDate	=>PIN_FLD_VERTEX_CTQ_DATE      	=>PIN_FLDT_TSTAMP
	* eCtqTime	=>PIN_FLD_VERTEX_CTQ_TIME      	=>PIN_FLDT_TSTAMP
	* eCtqTimestamp	=>PIN_FLD_VERTEX_CTQ_TIMESTAMP 	=>PIN_FLDT_TSTAMP
	* eCtqBool	=>PIN_FLD_VERTEX_CTQ_BOOL      	=>PIN_FLD_INT
	* eCtqChar	=>PIN_FLD_VERTEX_CTQ_CHAR      	=>PIN_FLDT_STR
	* eCtqEnum	=>PIN_FLD_VERTEX_CTQ_ENUM      	=>PIN_FLDT_ENUM
	*
	* YOU CAN ADD ONE FIELD LIKE THE FOLLOWING FLIST FORMAT.
	* 0 PIN_FLD_TAXES         ARRAY [] allocated 24, used 24
	* 1     PIN_FLD_FIELD_NAMES   ARRAY [] allocated 20, used 2
	* 2         PIN_FLD_TYPE           ENUM [0] 
	* 2         PIN_FLD_VERTEX_CTQ_STR    STR [0]  
	* USE PROPER MAPPING FIELD TO SET VALUE OF VERTEX FIELD.
	* IF YOU ADD SOME STRING FIELD(eCtqString),THEN SET VALUE FOR THAT FIELD
	* USING PIN_FLD_VERTEX_CTQ_STR.FOR DETAILS,SEE THE ABOVE TABLES.
	* 
	*
	* TO OVERRIDE DETAILED TAX INFORMATION (ATTRIBUTES THAT SET IN
	* REGTAX HANDEL IN VERTEX) SUCH AS TAX CODE, TAX TYPE, RATE, AND
	* TAX AUTHROITY, THE FIELD VALUES NEED TO SPECIFY UNDER THE
	* TAX_INPUT ARRAY, WITH INDEX STARTS FROM 1. 
	* EX:
	*  0 PIN_FLD_TAXES         ARRAY [] allocated 24, used 24
	*  ....
	*  1     PIN_FLD_TAX_INPUT     ARRAY [1] allocated 20, used 4
	*  2         PIN_FLD_FIELD_NAMES   ARRAY [340] allocated 20, used 2
	*  3             PIN_FLD_TYPE           ENUM [0] 10
	*  3             PIN_FLD_VERTEX_CTQ_STR    STR [0] "40"
	*  2         PIN_FLD_FIELD_NAMES   ARRAY [339] allocated 20, used 2
	*  3             PIN_FLD_TYPE           ENUM [0] 10
	*  3             PIN_FLD_VERTEX_CTQ_STR    STR [0] "0"
	*  2         PIN_FLD_FIELD_NAMES   ARRAY [341] allocated 20, used 2
	*  3             PIN_FLD_TYPE           ENUM [0] 10
	*  3             PIN_FLD_VERTEX_CTQ_STR    STR [0] "X"
	*  2         PIN_FLD_FIELD_NAMES   ARRAY [224] allocated 20, used 2
	*  3             PIN_FLD_TYPE           ENUM [0] 6
	*  3             PIN_FLD_VERTEX_CTQ_DOUBLE DECIMAL [0] 0
	*
	*
	* CUSTOMIZED FIELDS ARE SET INSIDE THE TAXES ARRAY ELEMENTS. WHEN THE 
	* CUSTOMIZATION FLAG IS SET, ONLY CUSTOMIZED FIELDS (FIELD_NAMES
	* UNDER TAXES AND FIELD_NAMES UNDER TAX_INPUT) ARE CONSIDERED. REST 
	* OF THE FIELDS ARE NEGLECTED.
	*
	******************************************************************/
	{	
	pin_flist_t     *cust_flistp = NULL;
        pin_flist_t     *field_name_flistp = NULL;
        tCtqAttribType  attribute_type = eCtqHandle ;
        time_t          now_t = 0;
        pin_decimal_t   *value_decimal = NULL;
	int32           value_int = 0;
        char            temp_buf[255] = { '\0' };
	pin_cookie_t    cookie = NULL;
	pin_flist_t     *t_flistp = NULL;
	pin_flist_t     *ti_flistp = NULL;
	int32           elemid = 0;
	
	int32           customized_flag = 1;/* SET TO 1 FOR CUSTOMIZATION */

	/***************************************************************** 
	 * IMPORTANT:SET customized_flag to 1 IF YOU WANT TO CUSTOMIZE THE  
	 * VERTEX FIELDS. 
	 *****************************************************************/	
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS_FLAGS,
 		(void *)&customized_flag, ebufp);
	

	while ((t_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistp, PIN_FLD_TAXES, 
		&elemid, 1, &cookie, ebufp)) != (pin_flist_t*)NULL) {

	        cust_flistp = PIN_FLIST_CREATE(ebufp);

	        /* FIELD:eCtqAttribCategoryCode NATURE:Mandatory Field */
         
	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                        eCtqAttribCategoryCode, ebufp);
	        attribute_type = eCtqString;
	        temp_buf[0]='\0';
	        strcpy(temp_buf ,"04") ;
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
		        (void *)&attribute_type,ebufp);
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_STR,
		        (void *)temp_buf,ebufp);

	        /* FIELD:eCtqAttribServiceCode NATURE:Mandatory Field */

	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                        eCtqAttribServiceCode, ebufp);

	        attribute_type = eCtqString;
	        temp_buf[0]='\0';
	        strcpy(temp_buf,"01" );
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
		        (void *)&attribute_type,ebufp);
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_STR,
		        (void *)temp_buf,ebufp);


	        /* FIELD:eCtqAttribOriginGeoCode NATURE:Mandatory Field */ 

	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                        eCtqAttribOriginGeoCode, ebufp);
	        attribute_type = eCtqString;
	        temp_buf[0]='\0';
	        strcpy(temp_buf ,"390290740" );
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
		        (void *)&attribute_type,ebufp);
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_STR,
		        (void *)temp_buf,ebufp);

	        /* FIELD:eCtqAttribTerminationGeoCode NATURE:Mandatory Field */

	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                        eCtqAttribTerminationGeoCode, ebufp);

	        attribute_type = eCtqString;
	        temp_buf[0]='\0';
	        strcpy(temp_buf,"390296350") ;
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
		        (void *)&attribute_type,ebufp);
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_STR,(void *)temp_buf,
		        ebufp);

	        /* FIELD:eCtqAttribInvoiceDate NATURE:Mandatory Field */

	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                	eCtqAttribInvoiceDate, ebufp);
	        attribute_type=eCtqDate;
	        now_t = pin_virtual_time((time_t *)NULL);
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
		        (void *)&attribute_type,ebufp);
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_DATE,&now_t,
		        ebufp);

	        /* FIELD:eCtqAttribTaxableAmount NATURE:Mandatory Field */

	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                        eCtqAttribTaxableAmount, ebufp);
	        attribute_type = eCtqDouble;
	        value_decimal = pbo_decimal_from_str("100.00",ebufp); 
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
		        (void *)&attribute_type,ebufp);
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_DOUBLE,
		        (void *)value_decimal,ebufp);

	        /**********************************************************************
	         * YOU CAN ADD OPTIONAL FIELDS LIKE BELOW FLIST
	         *
	         * field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, 
		        PIN_FLD_FIELD_NAMES,eCtqAttribBilledLines, ebufp);
	         * attribute_type = eCtqInt;
	         * PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
		        (void *)&attribute_type,ebufp);
	         * int value_int = 01
	         * PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_INT,
		        (void *)&value_int,ebufp);
	         **********************************************************************/

	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                        eCtqAttribBilledLines, ebufp);
	        attribute_type = eCtqInt;
	        value_int = 01; 
          	PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
		        (void *)&attribute_type,ebufp);
          	PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_INT,
		        (void *)&value_int,ebufp);

	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                        eCtqAttribSaleResaleCode, ebufp);
	        attribute_type = eCtqString;
	        temp_buf[0]='\0';
                strcpy(temp_buf ,"S" );
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
		        (void *)&attribute_type,ebufp);
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_STR,
		        temp_buf,ebufp);
	
	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                       eCtqAttribChargeToGeoCode, ebufp);
	        attribute_type = eCtqString;
                temp_buf[0]='\0';
	        strcpy(temp_buf ,"390296350" );
                PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
                        (void *)&attribute_type,ebufp);
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_STR,
		        temp_buf,ebufp);

	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                       eCtqAttribCallMinutes, ebufp);
	        attribute_type = eCtqDouble;
	        value_decimal = pbo_decimal_from_str("10.0",ebufp);
                PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
                        (void *)&attribute_type,ebufp);
                PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_DOUBLE,
                        (void *)value_decimal,ebufp);

	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                       eCtqAttribUserArea, ebufp);
	        attribute_type = eCtqString;
	        temp_buf[0]='\0';
	        strcpy(temp_buf ,"Interstate/Toll");
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
                        (void *)&attribute_type,ebufp);
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_STR,temp_buf,
		        ebufp);
	
	        field_name_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_FIELD_NAMES,
                      eCtqAttribInvoiceNumber, ebufp);
	        attribute_type = eCtqString;
	        temp_buf[0]='\0';
	        strcpy(temp_buf ,"DEMO-002");
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE,
                        (void *)&attribute_type,ebufp);
	        PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_STR,temp_buf,
                        ebufp);

            /**********************************************************************
             * YOU CAN OPTIONALLY OVERRIDE DETAILED TAX INFORMATION 
             * (ATTRIBUTES THAT SET IN REGTAX HANDEL IN VERTEX) SUCH AS 
             * TAX CODE, TAX TYPE, RATE, AND TAX AUTHROITY, THE FIELD 
             * VALUES NEED TO SPECIFY UNDER THE TAX_INPUT ARRAY, WITH 
             * INDEX STARTS FROM 1.			                         
             ***********************************************************************/
                ti_flistp = PIN_FLIST_ELEM_ADD(cust_flistp, PIN_FLD_TAX_INPUT, 1, ebufp);

                field_name_flistp = PIN_FLIST_ELEM_ADD(ti_flistp, PIN_FLD_FIELD_NAMES, 
                       eCtqAttribTaxType, ebufp);
                attribute_type = eCtqString; 
                temp_buf[0]='\0';
                strcpy(temp_buf,"40") ; 
                PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE, (void *)&attribute_type,ebufp); 
                PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_STR,(void *)temp_buf, ebufp);

                field_name_flistp = PIN_FLIST_ELEM_ADD(ti_flistp, PIN_FLD_FIELD_NAMES, 
                       eCtqAttribTaxAuthority, ebufp);
                attribute_type = eCtqString; 
                temp_buf[0]='\0';
                strcpy(temp_buf,"0") ; 
                PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE, (void *)&attribute_type,ebufp); 
                PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_STR,(void *)temp_buf, ebufp);

                field_name_flistp = PIN_FLIST_ELEM_ADD(ti_flistp, PIN_FLD_FIELD_NAMES,
                        eCtqAttribTaxCode, ebufp);
                attribute_type = eCtqString; 
                temp_buf[0]='\0';
                strcpy(temp_buf,"X") ; 
                PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE, (void *)&attribute_type,ebufp); 
                PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_STR,(void *)temp_buf, ebufp);

	        field_name_flistp = PIN_FLIST_ELEM_ADD(ti_flistp, PIN_FLD_FIELD_NAMES,
                       eCtqAttribRate, ebufp);
	        attribute_type = eCtqDouble;
	        value_decimal = pbo_decimal_from_str("0.0",ebufp);
                PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_TYPE, (void *)&attribute_type,ebufp);
                PIN_FLIST_FLD_SET(field_name_flistp,PIN_FLD_VERTEX_CTQ_DOUBLE, (void *)value_decimal,ebufp);





	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_rate_pol_pre_tax customized flist", cust_flistp);
	        PIN_FLIST_CONCAT(t_flistp,cust_flistp,ebufp);
	        if (value_decimal) {
                	pbo_decimal_destroy(&value_decimal);
	        }

	        PIN_FLIST_DESTROY_EX(&cust_flistp,NULL);
        }
	}
/***************************************************************************/
#endif

#ifdef ADD_CUSTOM_OUT_FLD_VERTEX_QSERIES
	/**********************************************************************
	* Uncomment the definition of the flag ADD_CUSTOM_OUT_FLD_VERTEX_QSERIES
	* at the beginning of the file for specifying  custom output fields 
	* be set in the output of  Vertex Commtax Q series.
	* Important : Also  uncomment the inclusion of header files stda.h, ctqa.h 
	* at the beginning of the file along with the flag 
	* ADD_CUSTOM_OUT_FLD_VERTEX_QSERIES
	**********************************************************************/
		if ( PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_TAXES, ebufp) > 0 )
		{

				pin_flist_t     *cust_out_res_flistp = NULL;
				pin_flist_t     *cust_out_flistp = NULL;

				pin_flist_t     *out_field_names_flistp = NULL;
				tCtqAttribType  out_attribute_type = NULL ;

				cust_out_res_flistp =  PIN_FLIST_ELEM_ADD(ret_flistp, 
										PIN_FLD_RESULTS , 0, ebufp);

			/**********************************************************************
			 * ADDITIONAL OUT FIELDS EITHER AT 
			 * PIN_FLD_TAXES LEVEL (Register Transaction data Handle) 
			 * OR PIN_FLD_SUBTOTAL LEVEL ( Register Transaction Tax Detail
			 * data Handle).
			 * CAN BE SPECIFIED USING THE CODE  SNIPPET AS BELOW
			 *
			 * out_field_names_flistp = PIN_FLIST_ELEM_ADD(cust_out_flistp, 
			 *    PIN_FLD_FIELD_NAMES,eCtqAttribBundleServiceCode, ebufp);
			 * out_attribute_type = eCtqString;
			 * PIN_FLIST_FLD_SET(out_field_names_flistp,PIN_FLD_TYPE,
					(void *)&out_attribute_type,ebufp);
			 **********************************************************************/
			cust_out_flistp = PIN_FLIST_ELEM_ADD(cust_out_res_flistp, 
										PIN_FLD_TAXES , 0, ebufp);

			out_field_names_flistp = PIN_FLIST_ELEM_ADD(cust_out_flistp, 
				 PIN_FLD_FIELD_NAMES,eCtqAttribBundleServiceCode, ebufp);
			out_attribute_type = eCtqString;
			PIN_FLIST_FLD_SET(out_field_names_flistp,PIN_FLD_TYPE,
					(void *)&out_attribute_type,ebufp);

			out_field_names_flistp = PIN_FLIST_ELEM_ADD(cust_out_flistp, 
					PIN_FLD_FIELD_NAMES,eCtqAttribTrunkLines, ebufp);
			out_attribute_type = eCtqInt;
			PIN_FLIST_FLD_SET(out_field_names_flistp,PIN_FLD_TYPE,
					(void *)&out_attribute_type,ebufp);

			cust_out_flistp = PIN_FLIST_ELEM_ADD(cust_out_res_flistp, 
										PIN_FLD_SUBTOTAL ,0, ebufp);
			
			out_field_names_flistp = PIN_FLIST_ELEM_ADD(cust_out_flistp, 
						PIN_FLD_FIELD_NAMES, eCtqAttribCustomerOverride, ebufp);
			out_attribute_type = eCtqString;
			PIN_FLIST_FLD_SET(out_field_names_flistp,PIN_FLD_TYPE,
					(void *)&out_attribute_type,ebufp);

		}
#endif


	/*
	 * Errors?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_rate_pol_pre_tax error", ebufp);
	} else {
		*o_flistpp = ret_flistp;
	}

	return;
}

