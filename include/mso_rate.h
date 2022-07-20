
typedef struct credit_threshold_config {
        int           resource_id;
        int           num_of_config;
        pin_decimal_t* threshold_config[20];
} credit_threshold_config_t;

#define NORMAL_LIM_PREPAID_USAGE_PROD 1020
#define DOCSIS2_LIM_PREPAID_USAGE_PROD 1050
#define DOCSIS3_LIM_PREPAID_USAGE_PROD 1080

typedef struct holiday_config {
	int	day;
	int	start_hr;
	int	end_hr;
} holiday_config_t;


typedef struct special_day_config {
	int	day;
	int 	month;
	int	start_hr;
	int	end_hr;
} special_day_config_t;
