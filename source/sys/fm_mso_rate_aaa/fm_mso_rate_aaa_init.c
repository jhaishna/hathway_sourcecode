/***********************************************************************
 *
 *      Copyright (c) 2000-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 ***********************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_mso_rate_aaa_init.c:BillingVelocityInt:3:2006-Sep-05 04:30:40 %";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pcm.h"
#include "pinlog.h"
#include "pin_errs.h"
#include "cm_fm.h"
#include "mso_rate.h"

/*******************************************************************
 * Global for holding credit threshold configuration
 *******************************************************************/
PIN_EXPORT holiday_config_t holiday_configs[7];
PIN_EXPORT int num_of_holidays_configured;

PIN_EXPORT special_day_config_t spcecial_day_configs[7];
PIN_EXPORT int num_of_special_days_configured;

void parse_config(char *str, int is_holiday);
void parse_holiday_config(char p[10][50],int num_of_holdiay_conf) ;
void parse_special_day_config(char p[10][50],int num_of_special_day_conf);

/***********************************************************************
 *Forward declaration
 ***********************************************************************/
EXPORT_OP void fm_mso_rate_aaa_init(int32 *errp);
char debug_msg[250];

/*******************************************************************
 * Routines defined elsewere.
 *******************************************************************/
/***********************************************************************
 *fm_mso_rate_aaa_init: One time initialization for fm_rate_pol
 ***********************************************************************/

void
fm_mso_rate_aaa_init(int32 *errp)
{
    int32                   errp1 = 0;
    char                    *str = NULL;	
    pin_flist_t		*bal_in_flistp = NULL;

	num_of_holidays_configured = 0;
	num_of_special_days_configured = 0;
	
	/***********************************************************
    * Read Holiday configuration and store them.
    ***********************************************************/	
    pin_conf("fm_mso", "holiday", PIN_FLDT_STR, (caddr_t*)&str, &errp1);    
    if (str != NULL)
    {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Holiday configuration found is ");
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,str);
		if(strlen(str) > 0 ) {
			parse_config(str, 1);
		}
    }

	


	/***********************************************************
    * Read Special day configuration and store them.
    ***********************************************************/	
    pin_conf("fm_mso", "special_day", PIN_FLDT_STR, (caddr_t*)&str, &errp1);    
	if (str != NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Special Day configuration found is ");
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,str);
		if(strlen(str) > 0 ) {
			parse_config(str, 0);
		}
	}
    

	
    return;
}

/**
 * 
 * config format will be
 * 
 * - fm_mso credit_threshold 1000009-70,80,90:1000010-70,80,90
 * 
 * 
 */
void parse_config(char *str, int is_holiday)
{
	int   num_of_conf = 0;	
	char temp_str[10][50];
	char *p  = strtok (str, ",");		
	while(p != NULL) {		
		memset(temp_str[num_of_conf],0,50);
		memcpy(temp_str[num_of_conf++], p, strlen(p));
		sprintf(debug_msg," Temp Str is %s\n", temp_str[num_of_conf-1]);
    		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		p = strtok(NULL, ",");
	}

	if(is_holiday) {
		parse_holiday_config(temp_str,num_of_conf);
		num_of_holidays_configured = num_of_conf;
	} else {
		parse_special_day_config(temp_str,num_of_conf);
		num_of_special_days_configured = num_of_conf;
	}
}

int get_day_of_week(char *day) {
	
	const char days[7][5] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	int i = 0;
    for(i = 0; i < 7; i++)
        if(strcmp(days[i],day) == 0)
            return i;
    return -1;
}

void parse_holiday_config(char p[10][50],int num_of_holdiay_conf) {
	
	int   num_of_percents_configured = 0;	
	pin_errbuf_t	ebuf;
	int i,j;
	char *c = NULL;
	char temp_str[5];
	PIN_ERRBUF_CLEAR(&ebuf);

	for (i = 0 ; i < num_of_holdiay_conf ; i++) {
		c = strtok (p[i], ":");	

		if(c != NULL) {
			sprintf(debug_msg," Setting day as %s",c);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			holiday_configs[i].day = get_day_of_week(c);
			sprintf(debug_msg," Setting day number as %d",holiday_configs[i].day);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		} else {
			return;
		}

		c = strtok(NULL, "-");
		sprintf(debug_msg," Setting start_hr as %s",c);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		holiday_configs[i].start_hr = atoi(c);
		c = strtok(NULL, "-");
		sprintf(debug_msg," Setting end_hr as %s",c);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		holiday_configs[i].end_hr = atoi(c);	
	}
}
void parse_special_day_config(char p[10][50],int num_of_special_day_conf) {
	
	pin_errbuf_t	ebuf;
	int i,j;
	char *c = NULL;
	char day[5];
	char month[5];
	char temp_str[5];
	PIN_ERRBUF_CLEAR(&ebuf);

	for (i = 0 ; i < num_of_special_day_conf ; i++) {
		strcpy(day,"");
		strcpy(month,"");
		c = strtok (p[i], ":");			
		if(c != NULL) {
			sprintf(debug_msg," Special Day read is %s",c);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			strncpy(day, c,  2);
			strncpy(month, c+2, 2);
			spcecial_day_configs[i].day = atoi(day);
			sprintf(debug_msg," Special Day read is %d",spcecial_day_configs[i].day);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			spcecial_day_configs[i].month = atoi(month);    
			sprintf(debug_msg," Special Day month read is %d",spcecial_day_configs[i].month);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		} else {
			return;
		}

		c = strtok(NULL, "-");
		sprintf(debug_msg," Setting start_hr as %s",c);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		spcecial_day_configs[i].start_hr = atoi(c);
		c = strtok(NULL, "-");
		sprintf(debug_msg," Setting end_hr as %s",c);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		spcecial_day_configs[i].end_hr = atoi(c);	
	}
}
