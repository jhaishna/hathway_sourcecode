/****************************************************
* CATV device obj types 
*****************************************************/
#define MSO_OBJ_TYPE_DEVICE_STB       "/device/stb"
#define MSO_OBJ_TYPE_DEVICE_VC 	      "/device/vc"
#define MSO_OBJ_TYPE_DEVICE_MODEM     "/device/modem"
#define MSO_OBJ_TYPE_DEVICE_ROUTER    "/device/router"
#define MSO_OBJ_TYPE_DEVICE_STATIC_IP "/device/ip"
#define MSO_OBJ_TYPE_DEVICE_ROUTER_CABLE    "/device/router/cable"
#define MSO_OBJ_TYPE_DEVICE_ROUTER_WIFI    "/device/router/wifi"
#define MSO_OBJ_TYPE_DEVICE_NAT       "/device/nat"
#define MSO_OBJ_TYPE_DEVICE_IP_STATIC "/device/ip/static"
#define MSO_OBJ_TYPE_DEVICE_IP_FRAMED "/device/ip/framed"
#define MSO_OBJ_TYPE_DEVICE_MC        "/device/mc"



/***********************************************
* STB device states 
***********************************************/
#define MSO_STB_STATE_GOOD 		1
#define MSO_STB_STATE_ALLOCATED 	2
#define MSO_STB_STATE_FAULTY		3
#define MSO_STB_STATE_REPAIRING		4
#define MSO_STB_STATE_DEAD 		5
#define MSO_STB_STATE_PREACTIVE 	6
#define MSO_STB_STATE_REPAIRED          9


/***********************************************
* VC device states
***********************************************/
#define MSO_VC_STATE_GOOD 		1
#define MSO_VC_STATE_ALLOCATED 		2
#define MSO_VC_STATE_FAULTY		3
#define MSO_VC_STATE_REPAIRING		4
#define MSO_VC_STATE_DEAD 		5
#define MSO_VC_STATE_PREACTIVE 		6
#define MSO_VC_STATE_BLACKLIST 		7
#define MSO_VC_STATE_REPAIRED           9


/***********************************************
* MODEM device states
***********************************************/
#define MSO_MODEM_STATE_GOOD 		1
#define MSO_MODEM_STATE_ALLOCATED 	2
#define MSO_MODEM_STATE_FAULTY		3
#define MSO_MODEM_STATE_REPAIRING	4
#define MSO_MODEM_STATE_DEAD 		5
#define MSO_MODEM_STATE_REPAIRED        9


/***********************************************
* ROUTER device states
***********************************************/
#define MSO_ROUTER_STATE_GOOD 		1
#define MSO_ROUTER_STATE_ALLOCATED 	2
#define MSO_ROUTER_STATE_FAULTY		3
#define MSO_ROUTER_STATE_REPAIRING	4
#define MSO_ROUTER_STATE_DEAD 		5
#define MSO_ROUTER_STATE_REPAIRED       9


/***********************************************
* NAT device states
***********************************************/
#define MSO_NAT_STATE_GOOD            1
#define MSO_NAT_STATE_ALLOCATED       2
#define MSO_NAT_STATE_FAULTY          3
#define MSO_NAT_STATE_REPAIRING       4
#define MSO_NAT_STATE_DEAD            5
#define MSO_NAT_STATE_REPAIRED        9


/***********************************************
* IP device states
***********************************************/
#define MSO_IP_STATE_GOOD               1
#define MSO_IP_STATE_ALLOCATED          2
#define MSO_IP_STATE_REPAIRED           9


/***********************************************
* MC device states
***********************************************/
#define MSO_MC_STATE_GOOD 		1
#define MSO_MC_STATE_ALLOCATED 		2
#define MSO_MC_STATE_FAULTY		3
#define MSO_MC_STATE_REPAIRING		4
#define MSO_MC_STATE_DEAD 		5


/***********************************************
* STATIC_IP device states
***********************************************/
#define MSO_STATIC_IP_STATE_AVAILABLE 		1
#define MSO_STATIC_IP_STATE_ALLOCATED 		2


/***********************************************
* Device Preactivation Status
***********************************************/
#define MSO_DEVICE_PREACTIVATION_NEW 		0
#define MSO_DEVICE_PREACTIVATION_ASSIGNED  1

/***********************************************
* Reason for Device Association 
*               and Disassociation 
***********************************************/
#define MSO_DEVICE_ACTIVATION_NEW                           1
#define MSO_DEVICE_ACTIVATION_REPLACEMENT                   3
#define MSO_DEVICE_DEACTIVATION_REASON_SERVICE_CLOSURE      2
#define MSO_DEVICE_DEACTIVATION_REASON_FAULTY               4
#define MSO_DEVICE_DEACTIVATION_REASON_TEMPORARY_DEVICE     6
#define MSO_DEVICE_DEACTIVATION_REASON_CHANGE_TECH	    8
#define MSO_DEVICE_DEACTIVATION_REASON_REPAIRED             7
#define MSO_DEVICE_ACTIVATION_REASON_REPAIRED               9
/***********************************************
* Reason for Device Swap 
***********************************************/
#define MSO_DEVICE_CHANGE_FAULTY_DEVICE		1
#define MSO_DEVICE_CHANGE_TEMPORARY_DEVICE		2
#define MSO_DEVICE_CHANGE_TECH_DEVICE		3

/********************************************
 * Preactivated service states
 ********************************************/
#define MSO_PREACTIVATED_SVC_ACTIVE	10100
#define MSO_PREACTIVATED_SVC_INACTIVE	10102
#define MSO_PREACTIVATED_SVC_CANCELLED	10103
#define MSO_PREACTIVATED_SVC_MOVED	10104


/**********************************************
* Device Lifecycle Flags
***********************************************/
#define CREATE		1
#define SET_STATE	2
#define ASSOCIATE	3
#define DISS		4
#define SET_ATT		5
#define MOV		6
#define REMOVE		7

/******************************************
Device types
******************************************/
#define DOCSIS2_CABLE_MODEM 10
#define DOCSIS3_CABLE_MODEM 20
#define DOCSIS2_WIFI_MODEM 11
#define DOCSIS3_WIFI_MODEM 21
#define OTT_MODEM_NO 43
#define OTT_MODEM "43"
#define OTT_MODEM_VAL 43
#define SUN_COUPON "42"
#define JIO_GPON_MODEM "35"
#define HW_GPON_MODEM "30"
#define HW_ONU_GPON_MODEM "31"

/*********************************************
Device plan types (value from OAP)
*********************************************/
#define MSO_DVC_TYPE_MODEM 1
#define MSO_DVC_TYPE_ROUTER_CABLE 2
#define MSO_DVC_TYPE_ROUTER_WIFI 3
#define MSO_DVC_TYPE_NAT 4 
#define MSO_DVC_TYPE_IP_STATIC 5 
#define MSO_DVC_TYPE_IP_FRAMED 6



