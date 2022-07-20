/*
 * Copyright (c) 2000, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 */

/*********************************************************************
 * This sample demonstrates how to call PCM_OP_TEST_LOOPBACK  opcode. 
 * The opcode simply echos back the flist that was passed to it.
 *********************************************************************/

#include <stdio.h>

#include "ops/base.h"
#include "ops/price.h"
#include "pcm.h"
#include "pinlog.h"

pin_flist_t* create_flist();


void create_bb_plan_report(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp
	); 

void create_catv_plan_report(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp
	); 

 void get_grant_product_values(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp
 ); 


void get_usage_product_values(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp
 );

 void get_purchase_product_values(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp
 );

  void get_rental_product_values(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp
 );

  void get_subscription_product_values(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	 char		*rateplan_name,
	pin_errbuf_t	*ebufp
 );
/******************************************************************
 * Main function to run the example
 ******************************************************************/
main(argc, argv)
	int	argc;
	char	**argv;
{
	pin_flist_t	*plan_search_iflistp = NULL;
	pin_flist_t	*plan_search_oflistp = NULL;
	pin_flist_t	*temp_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pcm_context_t	*ctxp;
	pin_errbuf_t        ebuf;
	int64               database;
	char		    *area_code_lco;
	char		search_template[100] = " select X from /plan where F1.type like V1 and name in (select plan_name from plan_master_list_t)";
	//char		search_template[100] = " select X from /plan where F1.type like V1 ";
	char		service_type[100];
	int		search_flags = 768;
	int64		db = 1;


	  if( argc == 3 )
	   {
	      printf("The argument supplied is %s\n", argv[0]);
	      printf("The argument supplied is %s\n", argv[1]);
	      printf("The argument supplied is %s\n", argv[2]);
	   }
	   else if( argc > 2 )
	   {
	      printf("Not correct arguments supplied exiting\n");
	      printf("Expected is: plan_report -service_type catv|bb\n");
	      return 1;
	   }

	/* Clear the error buffer */
	PIN_ERR_CLEAR_ERR(&ebuf);

	/* Get the input flist */
	//inflistp = create_flist();

	/* Print input flist */
	fprintf(stdout, "\nflist sent to server\n");
	if (PIN_ERR_IS_ERR(&ebuf)) {
	  fprintf(stderr, "error: couldn't display input flist\n");
	  exit(1);
	}
	strcpy( service_type,argv[2]);

	/* Open PCM connection. There must be a valid pin.conf
	 * file in the current directory. For more about context,
	 * see the context examples section.
	 */
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Opening context");
	
	PCM_CONNECT(&ctxp, &database, &ebuf);

	if (PIN_ERR_IS_ERR(&ebuf)) {
	  fprintf(stderr, "error opening pcm connection\n");
	  exit(1);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Context opened successfully");
	
	plan_search_iflistp = PIN_FLIST_CREATE(&ebuf);
	/* Search plan for the inut service */
	PIN_FLIST_FLD_PUT(plan_search_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, &ebuf), &ebuf);
	PIN_FLIST_FLD_SET(plan_search_iflistp, PIN_FLD_FLAGS, &search_flags, &ebuf);
	PIN_FLIST_FLD_SET(plan_search_iflistp, PIN_FLD_TEMPLATE, search_template, &ebuf);
	args_flistp = PIN_FLIST_ELEM_ADD(plan_search_iflistp, PIN_FLD_ARGS, 1, &ebuf);
	temp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, &ebuf);
	if(strcmp(service_type,"catv") == 0)
	{
		PIN_FLIST_FLD_PUT(temp_flistp, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/catv", -1, &ebuf), &ebuf);
	}
	else if (strcmp(service_type,"bb") == 0)
	{
		PIN_FLIST_FLD_PUT(temp_flistp, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/telco/broadband", -1, &ebuf), &ebuf);	
	}
	else 
	{
		printf("wrong service type in input\n");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "wrong service type in input");
		return 1;
	}

	results_flistp = PIN_FLIST_ELEM_ADD(plan_search_iflistp, PIN_FLD_RESULTS, 0, &ebuf);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID,NULL , &ebuf);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_NAME,NULL , &ebuf);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_DESCR,NULL , &ebuf);
	
	fprintf(stdout,"plan search input flist\n");
	PIN_FLIST_PRINT(plan_search_iflistp, NULL, &ebuf);

	//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " search account input list", plan_search_iflistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, plan_search_iflistp, &plan_search_oflistp, &ebuf);
	
	if (PIN_ERR_IS_ERR(&ebuf))
	{
			//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", &ebuf);
			fprintf(stdout,"Error in calling PCM_OP_SEARCH\n");
			PIN_ERR_CLEAR_ERR(&ebuf);
	}
	PIN_FLIST_DESTROY_EX(&plan_search_iflistp, NULL);
	//PIN_FLIST_DESTROY_EX(&plan_search_oflistp, NULL);
	fprintf(stdout,"plan search output flist\n");
	PIN_FLIST_PRINT(plan_search_oflistp, NULL, &ebuf);

	if(strcmp(service_type,"catv") == 0)
	{
		create_catv_plan_report(ctxp, plan_search_oflistp, &ebuf);
	}

	if(strcmp(service_type,"bb") == 0)
	{
		create_bb_plan_report(ctxp, plan_search_oflistp, &ebuf);
		
	}

	//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plan search account output flist", plan_search_oflistp);
	
	if (PIN_ERR_IS_ERR(&ebuf)) {
	  fprintf(stderr, "error calling opcode\n");
	  exit(1);
	}

	/* Process output */
//	 process_output(ctxp, outflistp, &ebuf);

	/* Close PCM connection. */
	PCM_CONTEXT_CLOSE(ctxp, (int32) 0, &ebuf);
	if (PIN_ERR_IS_ERR(&ebuf)) {
	  fprintf(stderr, "error closing pcm connection\n");
	  exit(1);
	}

	/* Print return flist */
	fprintf(stdout, "\nflist returned from server\n");
	//PIN_FLIST_PRINT(outflistp, NULL, &ebuf);
	if (PIN_ERR_IS_ERR(&ebuf)) {
	  fprintf(stderr, "error: couldn't output return flist\n");
	  exit(1);
	}
	return (0);
}

/*****************************************************************
 * Creates and returns flist of the form
 * 		0 PIN_FLD_POID                      POID [0] 0.0.0.1  -1 0
 *		0 PIN_FLD_LAST_NAME                  STR [0] "Mouse"
 *		0 PIN_FLD_FIRST_NAME                 STR [0] "Mickey"
 *****************************************************************/
pin_flist_t* create_flist() {
	pin_flist_t	    *flistp = NULL;
	pin_flist_t	    *s_flistp = NULL;
	pin_flist_t	    *res_flist = NULL;
	int64           database = 1;	
	char            *first = NULL;
	char            *last  = NULL;
	pin_errbuf_t    ebuf;
	poid_t          *a_pdp = NULL;
	pin_decimal_t	*st_amount = NULL;
	char		st[8];
	char 		flist_str[102400];

	/* Clear the error buffer */
	PIN_ERR_CLEAR_ERR(&ebuf);

	/* Allocate the flist */
	flistp = PIN_FLIST_CREATE(&ebuf);
	if (PIN_ERR_IS_ERR(&ebuf)) {
	  fprintf(stderr, "error creating flist");
	  exit(1);
	}

	/*******************************************************************
	 * PUT and SET
	 *
	 * Use PIN_FLIST_FLD_SET to add a copy of the field to an flist. 
	 * The value passed in does not have to be in dynamic memory and 
	 * it is unaffected by the macro.
	 *
	 * Use PIN_FLIST_FLD_PUT to add the field, including its data value 
	 * to an flist. The memory holding the value must be dynamically 
	 * allocated; useful to add a field to the flist without copying 
	 * its value. Once the value of the field has been added to the
	 * flist, the memory can no longer be accessed and the original 
	 * pointer is not valid.
	 * 
	 * Please see the flist documentation for more information.
	 ********************************************************************/

/*

sprintf(&flist_str[0], " \
0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 4                    \n\
1     PIN_FLD_PRIORITY     DECIMAL [0] 120                                \n\
1     PIN_FLD_POID           POID [0] 0.0.0.1 /product 245609179 17       \n\
1     PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 122408912 131 \n\
1     PIN_FLD_CONN_TYPE      ENUM [0] 1                                   \n\
0 PIN_FLD_RESULTS       ARRAY [1] allocated 20, used 4                    \n\
1     PIN_FLD_PRIORITY     DECIMAL [0] 130                                \n\
1     PIN_FLD_POID           POID [0] 0.0.0.1 /product 245611579 17       \n\
1     PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 122401023 133 \n\
1     PIN_FLD_CONN_TYPE      ENUM [0] 0                                   \n\
0 PIN_FLD_RESULTS       ARRAY [2] allocated 20, used 4 \n\
1     PIN_FLD_PRIORITY     DECIMAL [0] 120 \n\
1     PIN_FLD_POID           POID [0] 0.0.0.1 /product 245609179 17 \n\
1     PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 122400927 167 \n\
1     PIN_FLD_CONN_TYPE      ENUM [0] 1 \n\
");

PIN_STR_TO_FLIST(flist_str, database, &flistp, &ebuf );

s_flistp = PIN_FLIST_CREATE(&ebuf);
res_flist = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, &ebuf );
PIN_FLIST_FLD_SET(res_flist, PIN_FLD_CONN_TYPE, NULL , &ebuf);

PIN_FLIST_SORT(flistp, s_flistp, -1, &ebuf);

*/	
	printf("\nenter ST: ");
	scanf("%s", st); 

	a_pdp = PIN_POID_CREATE(database, "/mso_mta_job/device/stb", 11161672, &ebuf);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)a_pdp, &ebuf);

	st_amount = pbo_decimal_from_str(st, &ebuf);	
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_AMOUNT,
                        (void*)pbo_decimal_round(st_amount, 0, ROUND_UP, &ebuf), &ebuf );
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)a_pdp, &ebuf);
	
	if (PIN_ERR_IS_ERR(&ebuf)) {
	  fprintf(stderr, "error creating and adding poid");
	  exit(1);
	}
	
	/* Create and add two strings */ 
/*	first = "Mickey";
	last  = "Mouse";
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_FIRST_NAME, (void *)first, &ebuf);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_LAST_NAME, (void *)last, &ebuf);
	if (PIN_ERR_IS_ERR(&ebuf)) {
	  fprintf(stderr, "error creating and adding strings");
	  exit(1);
	}
*/

	return flistp;
}


void create_catv_plan_report(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp
 ) 
 {
	pin_flist_t    *flistp = NULL;
	int64           database = 1;	
	char            *first = NULL;
	char            *last  = NULL;
	poid_t          *a_pdp = NULL;

	pin_buf_t        *pin_bufp = NULL;
	char             *data = NULL;
	int32            len = 0;
	pin_flist_t      *inv_arr = NULL;



	/* Clear the error buffer */
	PIN_ERR_CLEAR_ERR(ebufp);
	fprintf(stdout, "\nflist returned from process_output\n");
	return ;
}


void create_bb_plan_report(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp
 ) 
 {
	pin_flist_t    *flistp = NULL;
	pin_flist_t    *args_flistp = NULL;
	pin_flist_t    *temp_flistp = NULL;
	pin_flist_t    *results_flistp = NULL;
	int64           database = 1;	
	char            *first = NULL;

	char            *last  = NULL;
	poid_t          *a_pdp = NULL;

	pin_buf_t        *pin_bufp = NULL;
	char             *data = NULL;
	int32            len = 0;
	
//Plan level info	
	int	elem_id_plan = 0;
	pin_cookie_t    cookie_plan = NULL;
	char		*plan_name = NULL;
	char		*plan_descr = NULL;
	poid_t		*plan_poid = NULL;
	int64		plan_id = 0;
	pin_flist_t	*plan_flistp = NULL;
// Deal level info
	int	elem_id_deal = 0;
	pin_cookie_t    cookie_deal = NULL;
	char		*deal_name = NULL;
	char		*deal_descr = NULL;
	poid_t          *deal_poid = NULL;
	int64		deal_id = 0;
	int		search_flags = 512;
	int64		db = 1;
	char		search_template_deal[256] = " select X from /deal 1 ,/plan 2 where 2.F1 = V1 and 1.F2 = 2.F3 ";
	pin_flist_t	*deal_search_iflistp = NULL;
	pin_flist_t	*deal_search_oflistp = NULL;
	pin_flist_t	*deal_flistp = NULL;

// product level info
	int	elem_id_product = 0;
	pin_cookie_t    cookie_product = NULL;
	char		*product_name = NULL;
	char		*product_descr = NULL;
	poid_t          *product_poid = NULL;
	int64		product_id = 0;
	char		search_template_product[256] = " select X from /product 1 ,/deal 2 where 2.F1 = V1 and 1.F2 = 2.F3 order by priority desc";
	pin_flist_t      *product_search_iflistp = NULL;
	pin_flist_t      *product_search_oflistp = NULL;
	pin_flist_t	 *product_flistp = NULL;
	
//get product info
	pin_flist_t      *get_product_iflistp = NULL;
	pin_flist_t      *get_product_oflistp = NULL;
	char		*event_type = NULL;
	char		*provisioning_tag = NULL;
	char		*tax_code = NULL;
	int32		*tax_when = NULL;
// grant product info
	pin_decimal_t	*grant_scaled_amount = NULL;
	pin_decimal_t	*grant_fixed_amount = NULL;
	int32		*grant_unit = NULL;
	int32		*validity_offset_value = NULL;
	int32		*validity_offset_unit = NULL;
	int32		*grant_prorate_first = NULL;
	int32		*grant_prorate_last = NULL;
// usage product info
	pin_decimal_t	*usage_rate = NULL;
	int32		*usage_unit = NULL;

// subscription & purchase product info
	pin_decimal_t	*fixed_amount = NULL;
	pin_decimal_t	*scaled_amount = NULL;
	pin_flist_t	*products_flistp = NULL;
	pin_flist_t	*usagemap_flistp = NULL;
	pin_flist_t	*rateplan_selector_flistp = NULL;
	pin_flist_t	*selector_flistp = NULL;
	pin_flist_t	*value_range_flistp = NULL;
	pin_flist_t	*value_range_city_flistp = NULL;
	pin_buf_t	*selector_buf = NULL;
	int32		*sb_prorate_first = NULL;
	int32		*sb_prorate_last = NULL;
	pin_decimal_t	*subscription_priority = NULL;
// Rateplan selector info
	int		elem_id_value_range = 0;
	pin_cookie_t	cookie_value_range = NULL;
	char		*city_name = NULL;
	char		*rateplan_name = NULL;
	pin_flist_t	*data_flistp = NULL;
	char		*bill_when = NULL;
	pin_flist_t	*column_1_flistp = NULL;
	char		*selector_fld1_name = NULL;
	pin_flist_t	*column_2_flistp = NULL;
	char		*selector_fld2_name = NULL;
	int		elem_id_city_value_range = 0;
	pin_cookie_t	cooki_city_evalue_range = NULL;
	int		sb_elem_id = 0;
	pin_cookie_t	sb_cookie = NULL;
	pin_flist_t	*rateplan_flistp = NULL;
	pin_flist_t	*rate_flistp = NULL;
	pin_flist_t	*quantity_tier_flistp = NULL;
	pin_flist_t	*ratebal_flistp = NULL;
	
	int32		*resource_id = NULL;
	FILE *fp;

	fp = fopen("bb_plan_report.csv", "w+");
	if( fp == NULL )
	{
		perror("Error while opening the file.\n");
		return;
	}

	/* Print header of the file*/
	fprintf(fp,"%s,","plan_name");
	fprintf(fp,"%s,","plan_descr");
	fprintf(fp,"%s,","plan_id");
	fprintf(fp,"%s,","deal_name");
	fprintf(fp,"%s,","deal_descr");
	fprintf(fp,"%s,","deal_id");
	fprintf(fp,"%s,","product_name");
	fprintf(fp,"%s,","product_descr");
	fprintf(fp,"%s,","product_id");
	fprintf(fp,"%s,","event_type");
	fprintf(fp,"%s,","grant_scaled_amount");
	fprintf(fp,"%s,","grant_fixed_amount");
	fprintf(fp,"%s,","grant_unit");
	fprintf(fp,"%s,","validity_offset_value");
	fprintf(fp,"%s,","validity_offset_unit");
	fprintf(fp,"%s,","grant_prorate_first");
	fprintf(fp,"%s,","grant_prorate_last");
	fprintf(fp,"%s,","usage_rate");
	fprintf(fp,"%s,","usage_unit");	
	fprintf(fp,"%s,","fixed_amount");
	fprintf(fp,"%s,","scaled_amount");
	fprintf(fp,"%s,","sb_prorate_first");
	fprintf(fp,"%s,","sb_prorate_last");
	fprintf(fp,"%s,","provisioning_tag");
	fprintf(fp,"%s,","subscription_priority");
	fprintf(fp,"%s,","tax_code");
	fprintf(fp,"%s,","tax_when");
	fprintf(fp,"%s,","city_name");
	fprintf(fp,"%s\n","bill_when");





	fprintf(stdout,"create_bb_plan_report input flist\n");
	PIN_FLIST_PRINT(in_flistp, NULL, ebufp);

	/* Clear the error buffer */
	PIN_ERR_CLEAR_ERR(ebufp);
	while ((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp,PIN_FLD_RESULTS,&elem_id_plan, 1,&cookie_plan, ebufp))!= NULL)
        {
		fprintf(stdout,"Plan result flist\n");
		PIN_FLIST_PRINT(plan_flistp, NULL, ebufp);

		plan_name = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_NAME , 1 , ebufp);
		plan_descr = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_DESCR , 1 , ebufp);
		plan_poid = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_POID , 1 , ebufp);

		deal_search_iflistp = PIN_FLIST_CREATE(ebufp);
		if (PIN_ERR_IS_ERR(ebufp))
		{
			fprintf(stderr, "error creating flist");
			fclose(fp);
			exit(1);
		}
		/* Search deals for the input plan */
		PIN_FLIST_FLD_PUT(deal_search_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(deal_search_iflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
		PIN_FLIST_FLD_SET(deal_search_iflistp, PIN_FLD_TEMPLATE, search_template_deal, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(deal_search_iflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID,plan_poid , ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(deal_search_iflistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID,NULL , ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(deal_search_iflistp, PIN_FLD_ARGS, 3, ebufp);
		temp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_DEAL_OBJ,NULL , ebufp);

		results_flistp = PIN_FLIST_ELEM_ADD(deal_search_iflistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID,NULL , ebufp);
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_NAME,NULL , ebufp);
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_DESCR,NULL , ebufp);
		fprintf(stdout,"deal search input flist\n");
		PIN_FLIST_PRINT(deal_search_iflistp, NULL, ebufp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, deal_search_iflistp, &deal_search_oflistp, ebufp);
	
		if (PIN_ERR_IS_ERR(ebufp))
		{
				//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
				fprintf(stdout,"Error in calling PCM_OP_SEARCH\n");
				PIN_ERR_CLEAR_ERR(ebufp);
		}
		PIN_FLIST_DESTROY_EX(&deal_search_iflistp, NULL);
		//PIN_FLIST_DESTROY_EX(&deal_search_oflistp, NULL);
		fprintf(stdout,"deal search output flist\n");
		PIN_FLIST_PRINT(deal_search_oflistp, NULL, ebufp);
		fprintf(stdout,"called search\n");
		
		elem_id_deal=0;
		cookie_deal=NULL;
		deal_name = NULL;
		deal_descr = NULL;
		deal_poid = NULL;
		while ((deal_flistp = PIN_FLIST_ELEM_GET_NEXT(deal_search_oflistp,PIN_FLD_RESULTS,&elem_id_deal, 1, &cookie_deal, ebufp))!= NULL)
		{
			deal_name = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_NAME , 1 , ebufp);
			deal_descr = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_DESCR , 1 , ebufp);
			deal_poid = PIN_FLIST_FLD_GET(deal_flistp, PIN_FLD_POID , 1 , ebufp);
			
			product_search_iflistp = PIN_FLIST_CREATE(ebufp);
			/* Search products for the  deal */
			PIN_FLIST_FLD_PUT(product_search_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(product_search_iflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
			PIN_FLIST_FLD_SET(product_search_iflistp, PIN_FLD_TEMPLATE, search_template_product, ebufp);
			
			args_flistp = PIN_FLIST_ELEM_ADD(product_search_iflistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID,deal_poid , ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(product_search_iflistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID,NULL , ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(product_search_iflistp, PIN_FLD_ARGS, 3, ebufp);			
			temp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
			PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_PRODUCT_OBJ,NULL , ebufp);

			results_flistp = PIN_FLIST_ELEM_ADD(product_search_iflistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID,NULL , ebufp);
			PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_NAME,NULL , ebufp);
			PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_DESCR,NULL , ebufp);
			temp_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp);
			PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_EVENT_TYPE,NULL , ebufp);
			
			fprintf(stdout,"Product search input flist\n");
			PIN_FLIST_PRINT(product_search_iflistp, NULL, ebufp);

			PCM_OP(ctxp, PCM_OP_SEARCH, 0, product_search_iflistp, &product_search_oflistp, ebufp);
		
			if (PIN_ERR_IS_ERR(ebufp))
			{
				//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
				fprintf(stdout,"Error in calling PCM_OP_SEARCH\n");
				PIN_ERR_CLEAR_ERR(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&deal_search_iflistp, NULL);
			//PIN_FLIST_DESTROY_EX(&deal_search_oflistp, NULL);
			fprintf(stdout,"Product search output flist\n");
			PIN_FLIST_PRINT(product_search_oflistp, NULL, ebufp);	
			
			elem_id_product=0;
			cookie_product=NULL;
			product_name = NULL;
			product_poid = NULL;
			event_type = NULL;
			
			provisioning_tag = NULL;
			tax_code = NULL;
			tax_when = NULL;

			grant_scaled_amount = NULL;
			grant_fixed_amount = NULL;
			grant_unit = NULL;
			validity_offset_value = NULL;
			validity_offset_unit = NULL;
			grant_prorate_first = NULL;
			grant_prorate_last = NULL;
			
			usage_rate = NULL;
			usage_unit = NULL;

			city_name = NULL;
			bill_when = NULL;

			sb_prorate_first = NULL;
			sb_prorate_last = NULL;
			fixed_amount = NULL;
			scaled_amount = NULL;

			while ((product_flistp = PIN_FLIST_ELEM_GET_NEXT(product_search_oflistp,PIN_FLD_RESULTS,&elem_id_product, 1, &cookie_product, ebufp))!= NULL)
			{
				product_name = PIN_FLIST_FLD_GET(product_flistp, PIN_FLD_NAME , 1 , ebufp);
				product_descr = PIN_FLIST_FLD_GET(product_flistp, PIN_FLD_DESCR , 1 , ebufp);
				product_poid = PIN_FLIST_FLD_GET(product_flistp, PIN_FLD_POID , 1 , ebufp);
				subscription_priority = PIN_FLIST_FLD_GET(product_flistp, PIN_FLD_PRIORITY , 1 , ebufp);
				
				temp_flistp = PIN_FLIST_ELEM_GET(product_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY,0,ebufp);
				event_type = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_EVENT_TYPE,0, ebufp);


				
				get_product_iflistp = PIN_FLIST_CREATE(ebufp);
				/* Get product data */
				PIN_FLIST_FLD_SET(get_product_iflistp, PIN_FLD_POID,product_poid , ebufp);
				PIN_FLIST_FLD_SET(get_product_iflistp, PIN_FLD_NAME,product_name , ebufp);
				fprintf(stdout,"product get data input flist\n");
				PIN_FLIST_PRINT(get_product_iflistp, NULL, ebufp);

				//PCM_OP(ctxp, PCM_OP_SEARCH, 0, product_search_iflistp, &product_search_oflistp, ebufp);
				PCM_OP(ctxp, PCM_OP_PRICE_GET_PRODUCT_INFO, 0,get_product_iflistp,&get_product_oflistp, ebufp);
			
				if (PIN_ERR_IS_ERR(ebufp))
				{
						//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
					fprintf(stdout,"Error in calling PCM_OP_SEARCH\n");
					PIN_ERR_CLEAR_ERR(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&get_product_iflistp, NULL);
				//PIN_FLIST_DESTROY_EX(&deal_search_oflistp, NULL);
				fprintf(stdout,"product get data output flist\n");
				PIN_FLIST_PRINT(get_product_oflistp, NULL, ebufp);
				
				if(strstr(event_type,"grant"))
				{
					get_grant_product_values(ctxp, get_product_oflistp, ebufp);
					if (PIN_ERR_IS_ERR(ebufp))
					{
						fprintf(stdout,"Error in calling PCM_OP_SEARCH\n");
						PIN_ERR_CLEAR_ERR(ebufp);
					}
					grant_scaled_amount = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_SCALED_AMOUNT,1, ebufp);
					grant_fixed_amount = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_FIXED_AMOUNT,1, ebufp);
					grant_unit = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_SCALED_UNIT,1, ebufp);
					validity_offset_value = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_RELATIVE_END_OFFSET,1, ebufp);
					validity_offset_unit = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_RELATIVE_END_UNIT,1, ebufp);
					grant_prorate_first = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_PRORATE_FIRST,1, ebufp);
					grant_prorate_last = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_PRORATE_LAST,1, ebufp);
						
				}
				else if (strstr(event_type,"/event/session/telco/broadband"))
				{
					get_usage_product_values(ctxp, get_product_oflistp, ebufp);
					if (PIN_ERR_IS_ERR(ebufp))
					{
						fprintf(stdout,"Error in calling PCM_OP_SEARCH\n");
						PIN_ERR_CLEAR_ERR(ebufp);
					}
					usage_rate = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_SCALED_AMOUNT,1, ebufp);
					usage_unit = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_SCALED_UNIT,1, ebufp);

				}
				else if (strstr(event_type,"purchase"))
				{
					get_purchase_product_values(ctxp, get_product_oflistp, ebufp);
					if (PIN_ERR_IS_ERR(ebufp))
					{
							fprintf(stdout,"Error in calling PCM_OP_SEARCH\n");
							PIN_ERR_CLEAR_ERR(ebufp);
					}
					fixed_amount = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_FIXED_AMOUNT,1, ebufp);
					scaled_amount = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_SCALED_AMOUNT,1, ebufp);
					tax_code = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_TAX_CODE,0, ebufp);
					tax_when = PIN_FLIST_FLD_GET(get_product_oflistp, PIN_FLD_TAX_WHEN,0, ebufp);

					// write the object here
					fprintf(fp,"%s,",plan_name);
					fprintf(fp,"%s,",plan_descr);
					fprintf(fp,"%d,",PIN_POID_GET_ID(plan_poid));
					fprintf(fp,"%s,",deal_name);
					fprintf(fp,"%s,",deal_descr);
					fprintf(fp,"%d,",PIN_POID_GET_ID(deal_poid));
					fprintf(fp,"%s,",product_name);
					fprintf(fp,"%s,",product_descr);
					fprintf(fp,"%d,",PIN_POID_GET_ID(product_poid));
					fprintf(fp,"%s,",event_type);


					if(grant_scaled_amount)
					{
						fprintf(fp,"%s,",pbo_decimal_to_str(grant_scaled_amount,ebufp));
					}
					else
					{
						fprintf(fp,"%s,","0.0");
					}

					if(grant_fixed_amount)
					{
						fprintf(fp,"%s,",pbo_decimal_to_str(grant_fixed_amount,ebufp));
					}
					else
					{
						fprintf(fp,"%s,","0.0");
					}


					if(grant_unit)
					{
						fprintf(fp,"%d,",*grant_unit);
					}
					else
					{
						fprintf(fp,"%s,","0");
					}

					if(validity_offset_value)
					{
						fprintf(fp,"%d,",*validity_offset_value);
					}
					else
					{
						fprintf(fp,"%s,","0");
					}

					if(validity_offset_unit)
					{
						fprintf(fp,"%d,",*validity_offset_unit);
					}
					else
					{
						fprintf(fp,"%s,","0");
					}

					if(grant_prorate_first)
					{
						fprintf(fp,"%d,",*grant_prorate_first);
					}
					else
					{
						fprintf(fp,"%s,","0");
					}

					if(grant_prorate_last)
					{
						fprintf(fp,"%d,",*grant_prorate_last);
					}
					else
					{
						fprintf(fp,"%s,","0");
					}

					if(usage_rate)
					{
						fprintf(fp,"%s,",pbo_decimal_to_str(usage_rate,ebufp));
					}
					else
					{
						fprintf(fp,"%s,","0.0");
					}

					if(usage_unit)
					{
						fprintf(fp,"%d,",*usage_unit);
					}
					else
					{
						fprintf(fp,"%s,","0");
					}

					if(fixed_amount)
					{
						fprintf(fp,"%s,",pbo_decimal_to_str(fixed_amount,ebufp));
					}
					else
					{
						fprintf(fp,"%s,","0.0");
					}

					if(scaled_amount)
					{
						fprintf(fp,"%s,",pbo_decimal_to_str(scaled_amount,ebufp));
					}
					else
					{
						fprintf(fp,"%s,","0.0");
					}

					if(sb_prorate_first)
					{
						fprintf(fp,"%d,",*sb_prorate_first);
					}
					else
					{
						fprintf(fp,"%s,","0");
					}

					if(sb_prorate_last)
					{
						fprintf(fp,"%d,",*sb_prorate_last);
					}
					else
					{
						fprintf(fp,"%s,","0");
					}
					if(provisioning_tag)
					{
						fprintf(fp,"%s,",provisioning_tag);
					}
					else
					{
						fprintf(fp,"%s","");
					}

					if(subscription_priority)
					{
						fprintf(fp,"%s,",pbo_decimal_to_str(subscription_priority,ebufp));
					}
					else
					{
						fprintf(fp,"%s,","0");
					}

					if(tax_code)
					{
						fprintf(fp,"%s,",tax_code);
					}
					else
					{
						fprintf(fp,"%s,","");
					}

					if(tax_when)
					{
						fprintf(fp,"%d,",*tax_when);
					}
					else
					{
						fprintf(fp,"%s\n","0");
					}


					if(city_name)
					{
						fprintf(fp,"%s,",city_name);
					}
					else
					{
						fprintf(fp,"%s,","");
					}

	
					if(bill_when)
					{
						fprintf(fp,"%s\n",bill_when);
					}
					else
					{
						fprintf(fp,"%s\n","0");
					}
					

				}
				else if (strstr(event_type,"/event/billing/product/fee/cycle")) //cycle event
				{
					
					fprintf(stdout,"Subscription product\n");
					products_flistp = PIN_FLIST_ELEM_GET(get_product_oflistp, PIN_FLD_PRODUCTS,PIN_ELEMID_ANY, 0, ebufp);
					fprintf(stdout,"subscription product products_flistp \n");
					PIN_FLIST_PRINT(products_flistp, NULL, ebufp);
					provisioning_tag = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PROVISIONING_TAG , 1 , ebufp);
					subscription_priority = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PRIORITY , 1 , ebufp);
					
					usagemap_flistp = PIN_FLIST_ELEM_GET(products_flistp, PIN_FLD_USAGE_MAP,PIN_ELEMID_ANY, 0, ebufp);
					
					fprintf(stdout,"subscription product usagemap_flistp \n");
					PIN_FLIST_PRINT(usagemap_flistp, NULL, ebufp);

					rateplan_selector_flistp = PIN_FLIST_SUBSTR_GET(usagemap_flistp, PIN_FLD_RATE_PLAN_SELECTOR,1,ebufp);
					fprintf(stdout,"subscription product rateplan_selector_flistp \n");
					PIN_FLIST_PRINT(rateplan_selector_flistp, NULL, ebufp);

					if(rateplan_selector_flistp)
					{
						selector_buf = (pin_buf_t *)PIN_FLIST_FLD_GET(rateplan_selector_flistp, PIN_FLD_SELECTOR, 0, ebufp );
						fprintf(stdout, "\nprint buffer\n");
						fprintf(stdout, selector_buf->data);						
						PIN_STR_TO_FLIST(selector_buf->data, 1, &selector_flistp, ebufp );
						fprintf(stdout, "\nAfter PIN_STR_TO_FLIST\n");
						PIN_FLIST_PRINT(selector_flistp, NULL, ebufp);

						if(PIN_FLIST_ELEM_COUNT(selector_flistp,PIN_FLD_COLUMNS,ebufp) == 1)//only CITY wise selector
						{
							
							fprintf(stdout, "\nCITY is as RP selctor\n");
							elem_id_value_range=0;
							cookie_value_range=NULL;
							sb_prorate_first = NULL;
							sb_prorate_last = NULL;
							fixed_amount = NULL;
							scaled_amount = NULL;
							city_name= NULL;
							bill_when = NULL;
							rateplan_name = NULL;
							tax_code = NULL;
							tax_when = NULL;

							column_1_flistp = PIN_FLIST_ELEM_GET(selector_flistp, PIN_FLD_COLUMNS, 0, 1, ebufp );
							selector_fld1_name = PIN_FLIST_FLD_GET(column_1_flistp, PIN_FLD_FIELD_NAME,1, ebufp);
							fprintf(stdout, "\nPIN_FLD_FIELD_NAME1 = %s\n",selector_fld1_name);

							while ((value_range_flistp = PIN_FLIST_ELEM_GET_NEXT (selector_flistp,PIN_FLD_VALUE_RANGES, &elem_id_value_range, 1, &cookie_value_range, ebufp))!= NULL)
							{
								if(strcmp(selector_fld1_name,"SERVICE.MSO_FLD_BB_INFO.PIN_FLD_CITY") == 0)
								{
									city_name = PIN_FLIST_FLD_GET(value_range_flistp, PIN_FLD_VALUE,1, ebufp);
									fprintf(stdout, "\nFound1 city = %s\n",city_name);
								}
								if(strcmp(selector_fld1_name,"SERVICE.MSO_FLD_BB_INFO.PIN_FLD_BILL_WHEN") == 0)
								{
									bill_when = PIN_FLIST_FLD_GET(value_range_flistp, PIN_FLD_VALUE,1, ebufp);
									fprintf(stdout, "\nFound1 bill_when = %s\n",bill_when);
								}

								rateplan_name = PIN_FLIST_FLD_GET(value_range_flistp, PIN_FLD_RATE_PLAN_NAME,1, ebufp);
								fprintf(stdout, "\nRateplan name = %s\n",rateplan_name);
								//get plan value here
								get_subscription_product_values(ctxp, get_product_oflistp,rateplan_name,ebufp);
								data_flistp = PIN_FLIST_ELEM_TAKE(get_product_oflistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
								fixed_amount   = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_FIXED_AMOUNT,0, ebufp);
								scaled_amount  = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_SCALED_AMOUNT,0, ebufp);
								sb_prorate_first = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_PRORATE_FIRST,0, ebufp);
								sb_prorate_last = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_PRORATE_LAST,0, ebufp);
								
								tax_code = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_TAX_CODE,0, ebufp);
								tax_when = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_TAX_WHEN,0, ebufp);								
								
								// write the object here

								fprintf(fp,"%s,",plan_name);
								fprintf(fp,"%s,",plan_descr);
								fprintf(fp,"%d,",PIN_POID_GET_ID(plan_poid));
								fprintf(fp,"%s,",deal_name);
								fprintf(fp,"%s,",deal_descr);
								fprintf(fp,"%d,",PIN_POID_GET_ID(deal_poid));
								fprintf(fp,"%s,",product_name);
								fprintf(fp,"%s,",product_descr);
								fprintf(fp,"%d,",PIN_POID_GET_ID(product_poid));
								fprintf(fp,"%s,",event_type);


								if(grant_scaled_amount)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(grant_scaled_amount,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0.0");
								}

								if(grant_fixed_amount)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(grant_fixed_amount,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0.0");
								}


								if(grant_unit)
								{
									fprintf(fp,"%d,",*grant_unit);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(validity_offset_value)
								{
									fprintf(fp,"%d,",*validity_offset_value);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(validity_offset_unit)
								{
									fprintf(fp,"%d,",*validity_offset_unit);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(grant_prorate_first)
								{
									fprintf(fp,"%d,",*grant_prorate_first);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(grant_prorate_last)
								{
									fprintf(fp,"%d,",*grant_prorate_last);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(usage_rate)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(usage_rate,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0.0");
								}

								if(usage_unit)
								{
									fprintf(fp,"%d,",*usage_unit);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(fixed_amount)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(fixed_amount,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0.0");
								}

								if(scaled_amount)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(scaled_amount,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0.0");
								}

								if(sb_prorate_first)
								{
									fprintf(fp,"%d,",*sb_prorate_first);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(sb_prorate_last)
								{
									fprintf(fp,"%d,",*sb_prorate_last);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}
								
								if(provisioning_tag)
								{
									fprintf(fp,"%s,",provisioning_tag);
								}
								else
								{
									fprintf(fp,"%s,","");
								}
								
								if(subscription_priority)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(subscription_priority,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(tax_code)
								{
									fprintf(fp,"%s,",tax_code);
								}
								else
								{
									fprintf(fp,"%s,","");
								}

								if(tax_when)
								{
									fprintf(fp,"%d,",*tax_when);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(city_name)
								{
									fprintf(fp,"%s,",city_name);
								}
								else
								{
									fprintf(fp,"%s,","");
								}
								if(bill_when)
								{
									fprintf(fp,"%s\n",bill_when);
								}
								else
								{
									fprintf(fp,"%s\n","0");
								}

								if (PIN_ERR_IS_ERR(ebufp))
								{
									fprintf(stdout,"RP data issue\n");
									PIN_ERR_CLEAR_ERR(ebufp);
								}
								PIN_FLIST_DESTROY_EX(&data_flistp, NULL);
								
							}
						}
						else if ((PIN_FLIST_ELEM_COUNT(selector_flistp,PIN_FLD_COLUMNS,ebufp) == 2))
						{
							column_1_flistp = PIN_FLIST_ELEM_GET(selector_flistp, PIN_FLD_COLUMNS, 0, 1, ebufp );
							selector_fld1_name = PIN_FLIST_FLD_GET(column_1_flistp, PIN_FLD_FIELD_NAME,1, ebufp);
							fprintf(stdout, "\nPIN_FLD_FIELD_NAME1 = %s\n",selector_fld1_name);

							column_2_flistp = PIN_FLIST_ELEM_GET(selector_flistp, PIN_FLD_COLUMNS, 1, 1, ebufp );
							selector_fld2_name = PIN_FLIST_FLD_GET(column_2_flistp, PIN_FLD_FIELD_NAME,1, ebufp);
							fprintf(stdout, "\nPIN_FLD_FIELD_NAME2 = %s\n",selector_fld2_name);

							fprintf(stdout, "\nCITY & payterm is as RP selctor\n");
							elem_id_value_range=0;
							cookie_value_range=NULL;
							sb_prorate_first = NULL;
							sb_prorate_last = NULL;
							fixed_amount = NULL;
							scaled_amount = NULL;
							city_name= NULL;
							bill_when = NULL;
							tax_code = NULL;
							tax_when = NULL;

							while ((value_range_flistp = PIN_FLIST_ELEM_GET_NEXT (selector_flistp,PIN_FLD_VALUE_RANGES, &elem_id_value_range, 1, &cookie_value_range, ebufp))!= NULL)
							{
								if(strcmp(selector_fld1_name,"SERVICE.MSO_FLD_BB_INFO.PIN_FLD_CITY") == 0)
								{
									city_name = PIN_FLIST_FLD_GET(value_range_flistp, PIN_FLD_VALUE,1, ebufp);
									fprintf(stdout, "\nFound1 city = %s\n",city_name);
								}
								if(strcmp(selector_fld1_name,"SERVICE.MSO_FLD_BB_INFO.PIN_FLD_BILL_WHEN") == 0)
								{
									bill_when = PIN_FLIST_FLD_GET(value_range_flistp, PIN_FLD_VALUE,1, ebufp);
									fprintf(stdout, "\nFound1 bill_when = %s\n",bill_when);
								}

								elem_id_city_value_range=0;
								cooki_city_evalue_range=NULL;
								while ((value_range_city_flistp = PIN_FLIST_ELEM_GET_NEXT (value_range_flistp,PIN_FLD_VALUE_RANGES, &elem_id_city_value_range, 1, &cooki_city_evalue_range, ebufp))!= NULL)
								{
									if(strcmp(selector_fld2_name,"SERVICE.MSO_FLD_BB_INFO.PIN_FLD_CITY") == 0)
									{
										city_name = PIN_FLIST_FLD_GET(value_range_city_flistp, PIN_FLD_VALUE,1, ebufp);
										fprintf(stdout, "\nFound2 city = %s\n",city_name);
									}

									if(strcmp(selector_fld2_name,"SERVICE.MSO_FLD_BB_INFO.PIN_FLD_BILL_WHEN") == 0)
									{
										bill_when = PIN_FLIST_FLD_GET(value_range_city_flistp, PIN_FLD_VALUE,1, ebufp);
										fprintf(stdout, "\nFound2 bill_when = %s\n",bill_when);
									}

									rateplan_name = PIN_FLIST_FLD_GET(value_range_city_flistp, PIN_FLD_RATE_PLAN_NAME,1, ebufp);
									fprintf(stdout, "\nRateplan name = %s\n",rateplan_name);
									//get plan value here
									get_subscription_product_values(ctxp, get_product_oflistp,rateplan_name,ebufp);
									data_flistp = PIN_FLIST_ELEM_TAKE(get_product_oflistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
									fixed_amount  = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_FIXED_AMOUNT,0, ebufp);
									scaled_amount  = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_SCALED_AMOUNT,0, ebufp);
									sb_prorate_first = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_PRORATE_FIRST,0, ebufp);
									sb_prorate_last = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_PRORATE_LAST,0, ebufp);
									tax_code = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_TAX_CODE,0, ebufp);
									tax_when = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_TAX_WHEN,0, ebufp);
																
									// write the object here
									fprintf(fp,"%s,",plan_name);
									fprintf(fp,"%s,",plan_descr);
									fprintf(fp,"%d,",PIN_POID_GET_ID(plan_poid));
									fprintf(fp,"%s,",deal_name);
									fprintf(fp,"%s,",deal_descr);
									fprintf(fp,"%d,",PIN_POID_GET_ID(deal_poid));
									fprintf(fp,"%s,",product_name);
									fprintf(fp,"%s,",product_descr);
									fprintf(fp,"%d,",PIN_POID_GET_ID(product_poid));
									fprintf(fp,"%s,",event_type);


									if(grant_scaled_amount)
									{
										fprintf(fp,"%s,",pbo_decimal_to_str(grant_scaled_amount,ebufp));
									}
									else
									{
										fprintf(fp,"%s,","0.0");
									}

									if(grant_fixed_amount)
									{
										fprintf(fp,"%s,",pbo_decimal_to_str(grant_fixed_amount,ebufp));
									}
									else
									{
										fprintf(fp,"%s,","0.0");
									}


									if(grant_unit)
									{
										fprintf(fp,"%d,",*grant_unit);
									}
									else
									{
										fprintf(fp,"%s,","0");
									}

									if(validity_offset_value)
									{
										fprintf(fp,"%d,",*validity_offset_value);
									}
									else
									{
										fprintf(fp,"%s,","0");
									}

									if(validity_offset_unit)
									{
										fprintf(fp,"%d,",*validity_offset_unit);
									}
									else
									{
										fprintf(fp,"%s,","0");
									}

									if(grant_prorate_first)
									{
										fprintf(fp,"%d,",*grant_prorate_first);
									}
									else
									{
										fprintf(fp,"%s,","0");
									}

									if(grant_prorate_last)
									{
										fprintf(fp,"%d,",*grant_prorate_last);
									}
									else
									{
										fprintf(fp,"%s,","0");
									}

									if(usage_rate)
									{
										fprintf(fp,"%s,",pbo_decimal_to_str(usage_rate,ebufp));
									}
									else
									{
										fprintf(fp,"%s,","0.0");
									}

									if(usage_unit)
									{
										fprintf(fp,"%d,",*usage_unit);
									}
									else
									{
										fprintf(fp,"%s,","0");
									}

									if(fixed_amount)
									{
										fprintf(fp,"%s,",pbo_decimal_to_str(fixed_amount,ebufp));
									}
									else
									{
										fprintf(fp,"%s,","0.0");
									}

									if(scaled_amount)
									{
										fprintf(fp,"%s,",pbo_decimal_to_str(scaled_amount,ebufp));
									}
									else
									{
										fprintf(fp,"%s,","0.0");
									}

									if(sb_prorate_first)
									{
										fprintf(fp,"%d,",*sb_prorate_first);
									}
									else
									{
										fprintf(fp,"%s,","0");
									}

									if(sb_prorate_last)
									{
										fprintf(fp,"%d,",*sb_prorate_last);
									}
									else
									{
										fprintf(fp,"%s,","0");
									}
									if(provisioning_tag)
									{
										fprintf(fp,"%s,",provisioning_tag);
									}
									else
									{
										fprintf(fp,"%s,","");
									}

									if(subscription_priority)
									{
										fprintf(fp,"%s,",pbo_decimal_to_str(subscription_priority,ebufp));
									}
									else
									{
										fprintf(fp,"%s,","0");
									}

									if(tax_code)
									{
										fprintf(fp,"%s,",tax_code);
									}
									else
									{
										fprintf(fp,"%s,","");
									}

									if(tax_when)
									{
										fprintf(fp,"%d,",*tax_when);
									}
									else
									{
										fprintf(fp,"%s,","0");
					}

									if(city_name)
									{
										fprintf(fp,"%s,",city_name);
									}
									else
									{
										fprintf(fp,"%s,","");
									}
									if(bill_when)
									{
										fprintf(fp,"%s\n",bill_when);
									}
									else
									{
										fprintf(fp,"%s\n","0");
									}

									if (PIN_ERR_IS_ERR(ebufp))
									{
										fprintf(stdout,"RP data issue\n");
										PIN_ERR_CLEAR_ERR(ebufp);
									}
									PIN_FLIST_DESTROY_EX(&data_flistp, NULL);
								}
								if (PIN_ERR_IS_ERR(ebufp))
								{
									fprintf(stdout,"RP data issue\n");
									PIN_ERR_CLEAR_ERR(ebufp);
								}
												
							}						
						}
					
					}
					else
					{
						sb_prorate_first = NULL;
						sb_prorate_last = NULL;
						fixed_amount = NULL;
						scaled_amount = NULL;
						city_name= NULL;
						bill_when = NULL;
						rateplan_name = NULL;
						tax_code = NULL;
						tax_when = NULL;
						
						fprintf(stdout, "\nNO Rate Plan selector\n");
						rateplan_flistp = PIN_FLIST_ELEM_GET(products_flistp, PIN_FLD_RATE_PLANS,PIN_ELEMID_ANY, 1, ebufp);
						fprintf(stdout,"subscription product rateplan_flistp \n");
						PIN_FLIST_PRINT(rateplan_flistp, NULL, ebufp);
						rate_flistp = PIN_FLIST_ELEM_GET(rateplan_flistp, PIN_FLD_RATES,PIN_ELEMID_ANY, 1, ebufp);
						sb_prorate_first = PIN_FLIST_FLD_GET(rate_flistp, PIN_FLD_PRORATE_FIRST,1 , ebufp);

						tax_code = PIN_FLIST_FLD_GET(rateplan_flistp, PIN_FLD_TAX_CODE,0, ebufp);
						tax_when = PIN_FLIST_FLD_GET(rateplan_flistp, PIN_FLD_TAX_WHEN,0, ebufp);

						sb_prorate_last = PIN_FLIST_FLD_GET(rate_flistp, PIN_FLD_PRORATE_LAST,1 , ebufp);
						quantity_tier_flistp = PIN_FLIST_ELEM_GET(rate_flistp, PIN_FLD_QUANTITY_TIERS,PIN_ELEMID_ANY, 1, ebufp);
						if (PIN_ERR_IS_ERR(ebufp))
						{
							fprintf(stdout,"Not correct configuration\n");
							PIN_ERR_CLEAR_ERR(ebufp);
						}
						sb_elem_id=0;
						sb_cookie=NULL;
						while ((ratebal_flistp = PIN_FLIST_ELEM_GET_NEXT(quantity_tier_flistp,PIN_FLD_BAL_IMPACTS,&sb_elem_id, 1, &sb_cookie, ebufp))!= NULL)
						{

							resource_id = PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_ELEMENT_ID,356 , ebufp);
							if(*resource_id == 356)
							{
								fixed_amount = (pin_decimal_t*)PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_FIXED_AMOUNT,0 , ebufp);
								scaled_amount = (pin_decimal_t*)PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_SCALED_AMOUNT,0 , ebufp);
							
								// Write object
								fprintf(fp,"%s,",plan_name);
								fprintf(fp,"%s,",plan_descr);
								fprintf(fp,"%d,",PIN_POID_GET_ID(plan_poid));
								fprintf(fp,"%s,",deal_name);
								fprintf(fp,"%s,",deal_descr);
								fprintf(fp,"%d,",PIN_POID_GET_ID(deal_poid));
								fprintf(fp,"%s,",product_name);
								fprintf(fp,"%s,",product_descr);
								fprintf(fp,"%d,",PIN_POID_GET_ID(product_poid));
								fprintf(fp,"%s,",event_type);


								if(grant_scaled_amount)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(grant_scaled_amount,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0.0");
								}

								if(grant_fixed_amount)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(grant_fixed_amount,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0.0");
								}


								if(grant_unit)
								{
									fprintf(fp,"%d,",*grant_unit);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(validity_offset_value)
								{
									fprintf(fp,"%d,",*validity_offset_value);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(validity_offset_unit)
								{
									fprintf(fp,"%d,",*validity_offset_unit);
								}
									else
								{
									fprintf(fp,"%s,","0");
								}

								if(grant_prorate_first)
								{
									fprintf(fp,"%d,",*grant_prorate_first);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(grant_prorate_last)
								{
									fprintf(fp,"%d,",*grant_prorate_last);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(usage_rate)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(usage_rate,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0.0");
								}

								if(usage_unit)
								{
									fprintf(fp,"%d,",*usage_unit);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(fixed_amount)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(fixed_amount,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0.0");
								}

								if(scaled_amount)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(scaled_amount,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0.0");
								}

								if(sb_prorate_first)
								{
									fprintf(fp,"%d,",*sb_prorate_first);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(sb_prorate_last)
								{
									fprintf(fp,"%d,",*sb_prorate_last);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}
								if(provisioning_tag)
								{
									fprintf(fp,"%s,",provisioning_tag);
								}
								else
								{
									fprintf(fp,"%s,","");
								}

								if(subscription_priority)
								{
									fprintf(fp,"%s,",pbo_decimal_to_str(subscription_priority,ebufp));
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(tax_code)
								{
									fprintf(fp,"%s,",tax_code);
								}
								else
								{
									fprintf(fp,"%s,","");
								}

								if(tax_when)
								{
									fprintf(fp,"%d,",*tax_when);
								}
								else
								{
									fprintf(fp,"%s,","0");
								}

								if(city_name)
								{
									fprintf(fp,"%s,",city_name);
								}
								else
								{
									fprintf(fp,"%s,","");
								}
								if(bill_when)
								{
									fprintf(fp,"%s\n",bill_when);
								}
								else
								{
									fprintf(fp,"%s\n","0");
								}


							}
						}				
					}
	
					
				}

			}
		}
		

	}
	fprintf(stdout, "\nflist returned from process_output\n");


//	pin_bufp = (pin_buf_t *)PIN_FLIST_FLD_GET(inv_flistp, PIN_FLD_INPUT_FLIST, 0, ebufp );
//	fprintf(stdout, "\nprint buffer\n");
//	fprintf(stdout, pin_bufp->data);
	
	//PIN_STR_TO_FLIST(pin_bufp->data, 1, &flistp, ebufp );
	//fprintf(stdout, "\nAfter PIN_STR_TO_FLIST\n");
	//PIN_FLIST_PRINT(flistp, NULL, ebufp);



	fclose(fp);
	return ;
}


 void get_grant_product_values(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp
 )
 {
	pin_flist_t    *flistp = NULL;
	pin_flist_t    *args_flistp = NULL;
	pin_flist_t    *temp_flistp = NULL;
	pin_flist_t    *results_flistp = NULL;
	
	pin_flist_t	*product_flistp = NULL;
	pin_flist_t	*rateplan_flistp = NULL;
	pin_flist_t	*rate_flistp = NULL;
	pin_flist_t	*quantity_tier_flistp = NULL;
	pin_flist_t	*ratebal_flistp = NULL;
	int32		*prorate_first = NULL;
	int32		*prorate_last = NULL;
	pin_decimal_t	*total_grant = NULL;
	pin_decimal_t	*fixed_amount = NULL;
	pin_decimal_t	*scaled_amount = NULL;
	int	elem_id = 0;
	pin_cookie_t    cookie = NULL;
	int32		*resource_id = NULL;
	fprintf(stdout,"get_grant_product_values input flist\n");
	PIN_FLIST_PRINT(in_flistp, NULL, ebufp);

	/* Clear the error buffer */

	product_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PRODUCTS,PIN_ELEMID_ANY, 0, ebufp);
	fprintf(stdout,"get_grant_product_values product_flistp \n");
	PIN_FLIST_PRINT(product_flistp, NULL, ebufp);
	rateplan_flistp = PIN_FLIST_ELEM_GET(product_flistp, PIN_FLD_RATE_PLANS,PIN_ELEMID_ANY, 0, ebufp);
	fprintf(stdout,"get_grant_product_values rateplan_flistp \n");
	PIN_FLIST_PRINT(rateplan_flistp, NULL, ebufp);
	rate_flistp = PIN_FLIST_ELEM_GET(rateplan_flistp, PIN_FLD_RATES,PIN_ELEMID_ANY, 0, ebufp);
	fprintf(stdout,"get_grant_product_values rate_flistp \n");
	PIN_FLIST_PRINT(rate_flistp, NULL, ebufp);

	prorate_first = PIN_FLIST_FLD_GET(rate_flistp, PIN_FLD_PRORATE_FIRST,0 , ebufp);
	prorate_last = PIN_FLIST_FLD_GET(rate_flistp, PIN_FLD_PRORATE_LAST,0 , ebufp);
	quantity_tier_flistp = PIN_FLIST_ELEM_GET(rate_flistp, PIN_FLD_QUANTITY_TIERS,PIN_ELEMID_ANY,0, ebufp);
	fprintf(stdout,"get_grant_product_values quantity_tier_flistp \n");
	PIN_FLIST_PRINT(quantity_tier_flistp, NULL, ebufp);



	while ((ratebal_flistp = PIN_FLIST_ELEM_GET_NEXT(quantity_tier_flistp,PIN_FLD_BAL_IMPACTS,&elem_id, 1, &cookie, ebufp))!= NULL)
	{

		//ratebal_flistp = PIN_FLIST_ELEM_GET(rate_flistp, PIN_FLD_BAL_IMPACTS,PIN_ELEMID_ANY 0, ebufp);
		resource_id = PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_ELEMENT_ID,356 , ebufp);
		if(*resource_id == 356 || *resource_id == 1000103)
		{
		}
		else
		{
			PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_FIXED_AMOUNT,in_flistp, PIN_FLD_FIXED_AMOUNT, ebufp);
			PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_SCALED_AMOUNT,in_flistp, PIN_FLD_SCALED_AMOUNT, ebufp);
			PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_SCALED_UNIT,in_flistp, PIN_FLD_SCALED_UNIT, ebufp);
			PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_RELATIVE_END_UNIT,in_flistp, PIN_FLD_RELATIVE_END_UNIT, ebufp);
			PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_RELATIVE_END_OFFSET,in_flistp, PIN_FLD_RELATIVE_END_OFFSET, ebufp);
			PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_FIRST,in_flistp, PIN_FLD_PRORATE_FIRST, ebufp);
			PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_LAST,in_flistp, PIN_FLD_PRORATE_LAST, ebufp);		}
		}
//	if(ratebal_flistp1)
//	{
//		fprintf(stdout,"get_grant_product_values rate_flistp \n");
//		PIN_FLIST_PRINT(ratebal_flistp1, NULL, ebufp);
//
//		PIN_FLIST_FLD_COPY(ratebal_flistp1,PIN_FLD_SCALED_UNIT,in_flistp, PIN_FLD_SCALED_UNIT, ebufp);
//		PIN_FLIST_FLD_COPY(ratebal_flistp1,PIN_FLD_RELATIVE_END_UNIT,in_flistp, PIN_FLD_RELATIVE_END_UNIT, ebufp);
//		PIN_FLIST_FLD_COPY(ratebal_flistp1,PIN_FLD_RELATIVE_END_OFFSET,in_flistp, PIN_FLD_RELATIVE_END_OFFSET, ebufp);
//		PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_FIRST,in_flistp, PIN_FLD_PRORATE_FIRST, ebufp);
//		PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_LAST,in_flistp, PIN_FLD_PRORATE_LAST, ebufp);
//	}
//	if(ratebal_flistp2)
//	{
//		fprintf(stdout,"get_grant_product_values rate_flistp \n");
//		PIN_FLIST_PRINT(ratebal_flistp2, NULL, ebufp);		PIN_FLIST_FLD_COPY(ratebal_flistp2,PIN_FLD_SCALED_UNIT,in_flistp, PIN_FLD_SCALED_UNIT, ebufp);
//		PIN_FLIST_FLD_COPY(ratebal_flistp2,PIN_FLD_RELATIVE_END_UNIT,in_flistp, PIN_FLD_RELATIVE_END_UNIT, ebufp);
//		PIN_FLIST_FLD_COPY(ratebal_flistp2,PIN_FLD_RELATIVE_END_OFFSET,in_flistp, PIN_FLD_RELATIVE_END_OFFSET, ebufp);
//		PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_FIRST,in_flistp, PIN_FLD_PRORATE_FIRST, ebufp);
//		PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_LAST,in_flistp, PIN_FLD_PRORATE_LAST, ebufp);	
//	}
	//resource_id = PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_ELEMENT_ID,0 , ebufp);
//	fixed_amount = (pin_decimal_t*)PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_FIXED_AMOUNT,0 , ebufp);
//	scaled_amount = (pin_decimal_t*)PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_SCALED_AMOUNT,0 , ebufp);
	

//	total_grant = pbo_decimal_from_str("0.0",ebufp);
//	pbo_decimal_add_assign(total_grant, fixed_amount, ebufp);
//	pbo_decimal_add_assign(total_grant, scaled_amount, ebufp);

	 
//	PIN_FLIST_FLD_PUT(in_flistp, PIN_FLD_SCALED_AMOUNT,total_grant , ebufp);


	fprintf(stdout, "\nget_grant_product_values output\n");
	PIN_FLIST_PRINT(in_flistp, NULL, ebufp);
	return ;

 }


void get_usage_product_values(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp
 )
 {
	pin_flist_t    *flistp = NULL;
	pin_flist_t    *args_flistp = NULL;
	pin_flist_t    *temp_flistp = NULL;
	pin_flist_t    *results_flistp = NULL;
	
	pin_flist_t	*product_flistp = NULL;
	pin_flist_t	*rateplan_flistp = NULL;
	pin_flist_t	*rate_flistp = NULL;
	pin_flist_t	*quantity_tier_flistp = NULL;
	pin_flist_t	*ratebal_flistp = NULL;
	int32		*prorate_first = NULL;
	int32		*prorate_last = NULL;
	pin_decimal_t	*total_grant = NULL;
	pin_decimal_t	*fixed_amount = NULL;
	pin_decimal_t	*scaled_amount = NULL;
	int32		*resource_id = NULL;
	int	elem_id = 0;
	pin_cookie_t    cookie = NULL;
	int	elem_id1 = 0;
	pin_cookie_t    cookie1 = NULL;


	fprintf(stdout,"get_usage_product_values input flist\n");
	PIN_FLIST_PRINT(in_flistp, NULL, ebufp);

	/* Clear the error buffer */

	product_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PRODUCTS,PIN_ELEMID_ANY, 0, ebufp);
	rateplan_flistp = PIN_FLIST_ELEM_GET(product_flistp, PIN_FLD_RATE_PLANS,PIN_ELEMID_ANY, 0, ebufp);
	rate_flistp = PIN_FLIST_ELEM_GET(rateplan_flistp, PIN_FLD_RATES,PIN_ELEMID_ANY, 0, ebufp);
	prorate_first = PIN_FLIST_FLD_GET(rate_flistp, PIN_FLD_PRORATE_FIRST,0 , ebufp);
	prorate_last = PIN_FLIST_FLD_GET(rate_flistp, PIN_FLD_PRORATE_LAST,0 , ebufp);
	//quantity_tier_flistp = PIN_FLIST_ELEM_GET(rate_flistp, PIN_FLD_QUANTITY_TIERS,PIN_ELEMID_ANY, 0, ebufp);

	elem_id1=0;
	cookie1=NULL;
	while ((quantity_tier_flistp = PIN_FLIST_ELEM_GET_NEXT(rate_flistp,PIN_FLD_QUANTITY_TIERS,&elem_id1, 1, &cookie1, ebufp))!= NULL)
	{

		elem_id=0;
		cookie=NULL;
		while ((ratebal_flistp = PIN_FLIST_ELEM_GET_NEXT(quantity_tier_flistp,PIN_FLD_BAL_IMPACTS,&elem_id, 1, &cookie, ebufp))!= NULL)
		{

			//ratebal_flistp = PIN_FLIST_ELEM_GET(rate_flistp, PIN_FLD_BAL_IMPACTS,PIN_ELEMID_ANY 0, ebufp);
			resource_id = PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_ELEMENT_ID,356 , ebufp);
			if(*resource_id == 356)
			{
	//			fixed_amount = (pin_decimal_t*)PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_FIXED_AMOUNT,0 , ebufp);
	//			scaled_amount = (pin_decimal_t*)PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_SCALED_AMOUNT,0 , ebufp);
	//		
	//
	//			total_grant = pbo_decimal_from_str("0.0",ebufp);
	//			pbo_decimal_add_assign(total_grant, fixed_amount, ebufp);
	//			pbo_decimal_add_assign(total_grant, scaled_amount, ebufp);
	//
	//			 
	//			PIN_FLIST_FLD_PUT(in_flistp, PIN_FLD_SCALED_AMOUNT,total_grant , ebufp);
				PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_FIXED_AMOUNT,in_flistp, PIN_FLD_FIXED_AMOUNT, ebufp);
				PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_SCALED_AMOUNT,in_flistp, PIN_FLD_SCALED_AMOUNT, ebufp);
				PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_SCALED_UNIT,in_flistp, PIN_FLD_SCALED_UNIT, ebufp);
				PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_RELATIVE_END_UNIT,in_flistp, PIN_FLD_RELATIVE_END_UNIT, ebufp);
				PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_RELATIVE_END_OFFSET,in_flistp, PIN_FLD_RELATIVE_END_OFFSET, ebufp);
				PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_FIRST,in_flistp, PIN_FLD_PRORATE_FIRST, ebufp);
				PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_LAST,in_flistp, PIN_FLD_PRORATE_LAST, ebufp);
				PIN_FLIST_FLD_COPY(rateplan_flistp,PIN_FLD_TAX_CODE,in_flistp, PIN_FLD_TAX_CODE, ebufp);
				PIN_FLIST_FLD_COPY(rateplan_flistp,PIN_FLD_TAX_WHEN,in_flistp, PIN_FLD_TAX_WHEN, ebufp);
			}
		}
	}

	fprintf(stdout, "\nget_usage_product_values output\n");
	PIN_FLIST_PRINT(in_flistp, NULL, ebufp);
	return ;

 
 }

 void get_purchase_product_values(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp
 )
 {
	pin_flist_t    *flistp = NULL;
	pin_flist_t    *args_flistp = NULL;
	pin_flist_t    *temp_flistp = NULL;
	pin_flist_t    *results_flistp = NULL;
	
	pin_flist_t	*product_flistp = NULL;
	pin_flist_t	*rateplan_flistp = NULL;
	pin_flist_t	*rate_flistp = NULL;
	pin_flist_t	*quantity_tier_flistp = NULL;
	pin_flist_t	*ratebal_flistp = NULL;
	int32		*prorate_first = NULL;
	int32		*prorate_last = NULL;
	pin_decimal_t	*total_grant = NULL;
	pin_decimal_t	*fixed_amount = NULL;
	pin_decimal_t	*scaled_amount = NULL;
	int32		*resource_id = NULL;
	int	elem_id = 0;
	pin_cookie_t    cookie = NULL;


	fprintf(stdout,"get_purchase_product_values\n");
	PIN_FLIST_PRINT(in_flistp, NULL, ebufp);

	/* Clear the error buffer */

	product_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PRODUCTS,PIN_ELEMID_ANY, 0, ebufp);
	rateplan_flistp = PIN_FLIST_ELEM_GET(product_flistp, PIN_FLD_RATE_PLANS,PIN_ELEMID_ANY, 0, ebufp);
	rate_flistp = PIN_FLIST_ELEM_GET(rateplan_flistp, PIN_FLD_RATES,PIN_ELEMID_ANY, 0, ebufp);
	prorate_first = PIN_FLIST_FLD_GET(rate_flistp, PIN_FLD_PRORATE_FIRST,0 , ebufp);
	prorate_last = PIN_FLIST_FLD_GET(rate_flistp, PIN_FLD_PRORATE_LAST,0 , ebufp);
	quantity_tier_flistp = PIN_FLIST_ELEM_GET(rate_flistp, PIN_FLD_QUANTITY_TIERS,PIN_ELEMID_ANY, 0, ebufp);
	elem_id=0;
	cookie=NULL;
	while ((ratebal_flistp = PIN_FLIST_ELEM_GET_NEXT(quantity_tier_flistp,PIN_FLD_BAL_IMPACTS,&elem_id, 1, &cookie, ebufp))!= NULL)
	{

		//ratebal_flistp = PIN_FLIST_ELEM_GET(rate_flistp, PIN_FLD_BAL_IMPACTS,PIN_ELEMID_ANY 0, ebufp);
		resource_id = PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_ELEMENT_ID,356 , ebufp);
		if(*resource_id == 356)
		{
//			fixed_amount = (pin_decimal_t*)PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_FIXED_AMOUNT,0 , ebufp);
//
//
//			total_grant = pbo_decimal_from_str("0.0",ebufp);
//			pbo_decimal_add_assign(total_grant, fixed_amount, ebufp);
//		 
//			PIN_FLIST_FLD_PUT(in_flistp, PIN_FLD_FIXED_AMOUNT,total_grant , ebufp);
			PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_FIXED_AMOUNT,in_flistp, PIN_FLD_FIXED_AMOUNT, ebufp);
			PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_SCALED_AMOUNT,in_flistp, PIN_FLD_SCALED_AMOUNT, ebufp);
			PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_SCALED_UNIT,in_flistp, PIN_FLD_SCALED_UNIT, ebufp);
			PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_RELATIVE_END_UNIT,in_flistp, PIN_FLD_RELATIVE_END_UNIT, ebufp);
			PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_RELATIVE_END_OFFSET,in_flistp, PIN_FLD_RELATIVE_END_OFFSET, ebufp);
			PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_FIRST,in_flistp, PIN_FLD_PRORATE_FIRST, ebufp);
			PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_LAST,in_flistp, PIN_FLD_PRORATE_LAST, ebufp);
			PIN_FLIST_FLD_COPY(rateplan_flistp,PIN_FLD_TAX_CODE,in_flistp, PIN_FLD_TAX_CODE, ebufp);
			PIN_FLIST_FLD_COPY(rateplan_flistp,PIN_FLD_TAX_WHEN,in_flistp, PIN_FLD_TAX_WHEN, ebufp);
		}
	}

	fprintf(stdout, "\nget_purchase_product_values output\n");
	PIN_FLIST_PRINT(in_flistp, NULL, ebufp);
	return ; 
 }

  void get_subscription_product_values(
	pcm_context_t*	ctxp,
	pin_flist_t	*in_flistp,
	 char		*rateplan_name,
	pin_errbuf_t	*ebufp
 )
 {
 	pin_flist_t    *flistp = NULL;
	pin_flist_t    *args_flistp = NULL;
	pin_flist_t    *temp_flistp = NULL;
	pin_flist_t    *results_flistp = NULL;
	
	pin_flist_t	*product_flistp = NULL;
	pin_flist_t	*rateplan_flistp = NULL;
	pin_flist_t	*rate_flistp = NULL;
	pin_flist_t	*data_flistp = NULL;
	pin_flist_t	*quantity_tier_flistp = NULL;
	pin_flist_t	*ratebal_flistp = NULL;
	int32		*prorate_first = NULL;
	int32		*prorate_last = NULL;
	pin_decimal_t	*total_grant = NULL;
	pin_decimal_t	*fixed_amount = NULL;
	pin_decimal_t	*scaled_amount = NULL;
	char		*rateplan_name_lc=NULL;
	int32		*resource_id = NULL;
	int	elem_id = 0;
	pin_cookie_t    cookie = NULL;
	int	elem_id_rp =0;
	pin_cookie_t    cookie_rp=NULL;


	fprintf(stdout,"get_subscription_product_values\n");
	PIN_FLIST_PRINT(in_flistp, NULL, ebufp);

	/* Clear the error buffer */

	product_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PRODUCTS,PIN_ELEMID_ANY, 0, ebufp);
	fprintf(stdout,"get_subscription_product_values product_flistp\n");
	PIN_FLIST_PRINT(product_flistp, NULL, ebufp);

	elem_id_rp=0;
	cookie_rp=NULL;
	while ((rateplan_flistp = PIN_FLIST_ELEM_GET_NEXT(product_flistp,PIN_FLD_RATE_PLANS,&elem_id_rp, 1, &cookie_rp, ebufp))!= NULL)
	{
		//rateplan_flistp = PIN_FLIST_ELEM_GET(product_flistp, PIN_FLD_RATE_PLANS,PIN_ELEMID_ANY, 0, ebufp);
		fprintf(stdout,"get_subscription_product_values rateplan_flistp\n");
		fprintf(stdout,"get_subscription_product_values rateplan name %s\n",rateplan_name);
		PIN_FLIST_PRINT(rateplan_flistp, NULL, ebufp);
		rateplan_name_lc = PIN_FLIST_FLD_GET(rateplan_flistp, PIN_FLD_NAME,0 , ebufp);
		fprintf(stdout,"get_subscription_product_values rateplan name %s\n",rateplan_name_lc);
		if(strcmp(rateplan_name_lc,rateplan_name)==0)
		{
		fprintf(stdout,"get_subscription_product_values rateplan found\n");	
			rate_flistp = PIN_FLIST_ELEM_GET(rateplan_flistp, PIN_FLD_RATES,PIN_ELEMID_ANY, 0, ebufp);
			prorate_first = PIN_FLIST_FLD_GET(rate_flistp, PIN_FLD_PRORATE_FIRST,0 , ebufp);
			prorate_last = PIN_FLIST_FLD_GET(rate_flistp, PIN_FLD_PRORATE_LAST,0 , ebufp);
			quantity_tier_flistp = PIN_FLIST_ELEM_GET(rate_flistp, PIN_FLD_QUANTITY_TIERS,PIN_ELEMID_ANY, 0, ebufp);
			elem_id=0;
			cookie=NULL;
			while ((ratebal_flistp = PIN_FLIST_ELEM_GET_NEXT(quantity_tier_flistp,PIN_FLD_BAL_IMPACTS,&elem_id, 1, &cookie, ebufp))!= NULL)
			{

				//ratebal_flistp = PIN_FLIST_ELEM_GET(rate_flistp, PIN_FLD_BAL_IMPACTS,PIN_ELEMID_ANY 0, ebufp);
				resource_id = PIN_FLIST_FLD_GET(ratebal_flistp, PIN_FLD_ELEMENT_ID,356 , ebufp);
				if(*resource_id == 356)
				{
					data_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp );
				 
					PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_FIXED_AMOUNT,data_flistp, PIN_FLD_FIXED_AMOUNT, ebufp);
					PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_SCALED_AMOUNT,data_flistp, PIN_FLD_SCALED_AMOUNT, ebufp);
					PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_SCALED_UNIT,data_flistp, PIN_FLD_SCALED_UNIT, ebufp);
					PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_RELATIVE_END_UNIT,data_flistp, PIN_FLD_RELATIVE_END_UNIT, ebufp);
					PIN_FLIST_FLD_COPY(ratebal_flistp,PIN_FLD_RELATIVE_END_OFFSET,data_flistp, PIN_FLD_RELATIVE_END_OFFSET, ebufp);
					PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_FIRST,data_flistp, PIN_FLD_PRORATE_FIRST, ebufp);
					PIN_FLIST_FLD_COPY(rate_flistp,PIN_FLD_PRORATE_LAST,data_flistp, PIN_FLD_PRORATE_LAST, ebufp);
					PIN_FLIST_FLD_COPY(rateplan_flistp,PIN_FLD_TAX_CODE,data_flistp, PIN_FLD_TAX_CODE, ebufp);
					PIN_FLIST_FLD_COPY(rateplan_flistp,PIN_FLD_TAX_WHEN,data_flistp, PIN_FLD_TAX_WHEN, ebufp);
				}
			}
		}
		else
		{
					fprintf(stdout,"get_subscription_product_values rateplan not found\n");		
		}
	}

	fprintf(stdout, "\nget_subscription_product_values output\n");
	PIN_FLIST_PRINT(in_flistp, NULL, ebufp);
	return ; 
 }
