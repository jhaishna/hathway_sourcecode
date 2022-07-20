#include <stdio.h>
#include <string.h>

typedef struct credit_threshold_config {
        int           resource_id;           
        int           num_of_config;           
        char*           threshold_config[20];  
} credit_threshold_config_t;


credit_threshold_config_t credit_thresholds[10];
int num_of_resources_configured = 0;

void parse_credit_threshold_config(char p[10][50],int num_of_resources_configured, credit_threshold_config_t credit_thresholds[20]) {
	
	int   num_of_percents_configured = 0;	
	int i;

	for (i = 0 ; i < num_of_resources_configured ; i++) {
	char *c    = strtok (p[i], "-,");	

	if(c != NULL) {
		printf(" Setting resource id as %s\n",c);
		credit_thresholds[num_of_resources_configured].resource_id = atoi(c);
	} else {
		return;
	}
	c = strtok(NULL, ",");
	
	while(c != NULL) {		
		printf("Setting threshold as %s\n",c);
		credit_thresholds[num_of_resources_configured].threshold_config[num_of_percents_configured++] = c;
		c = strtok(NULL, ",");
	}
	credit_thresholds[num_of_resources_configured].num_of_config = num_of_percents_configured;
	printf("Credit Threshold parse for this resource complete!\n");

	}
	
}

/**
 * 
 * config format will be
 * 
 * - fm_mso credit_threshold 1000009-70,80,90:1000010-70,80,90
 * 
 * 
 */
void split_token(char *str, credit_threshold_config_t credit_thresholds[10])
{
	int   num_of_resources_configured = 0;	
	char temp_str[10][50];
	char *p  = strtok (str, ":");		
	while(p != NULL) {		
		printf(" Split after : is %s\n", p);
		memset(temp_str[num_of_resources_configured],0,50);
		memcpy(temp_str[num_of_resources_configured++], p, strlen(p));
		printf(" Temp Str is %s\n", temp_str[num_of_resources_configured-1]);
		p = strtok(NULL, ":");
	}
	parse_credit_threshold_config(temp_str,num_of_resources_configured, credit_thresholds);
}

int main(int argc, char *argv[])
{
	char str[100] = "1000031-05,10,15:1000032-25,30,45";
	printf(" First print.. %s\n",str);
	split_token(str, credit_thresholds);	
	return 0;
}
