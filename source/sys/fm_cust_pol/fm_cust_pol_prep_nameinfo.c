/*******************************************************************
 *
* Copyright (c) 1999, 2015, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_prep_nameinfo.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 04:29:52 nishahan Exp $";
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "pin_pymt.h"
#include "psiu_business_params.h"

EXPORT_OP void
op_cust_pol_prep_nameinfo(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);


static void
fm_cust_pol_prep_nameinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

extern void
fm_cust_pol_map_country(
        pin_flist_t     *flistp,
        pin_flist_t     *r_flistp, 
        char            *country,
        pin_errbuf_t    *ebufp);

static void
fm_cust_pol_prep_nameinfo_psm(
	pcm_context_t	*ctxp,
	poid_t			*a_pdp,
	pin_flist_t		*in_flistp,
	pin_errbuf_t	*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_PREP_NAMEINFO  command
 *******************************************************************/
void
op_cust_pol_prep_nameinfo(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_PREP_NAMEINFO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_prep_nameinfo", ebufp);
		return;
	}

	/***********************************************************
	 * Log the input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_prep_nameinfo input flist", in_flistp);

	/***********************************************************
	 * We will not open any transactions with Policy FM
	 * since policies should NEVER modify the database.
	 ***********************************************************/

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_prep_nameinfo(ctxp, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_prep_nameinfo error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_prep_nameinfo return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_prep_nameinfo()
 *
 *	Prep the nameinfo to be ready for on-line registration.
 *
 *	Preparations performed include:
 *		- default country is USA
 *		- payment suspense account
 *
 *******************************************************************/
static void
fm_cust_pol_prep_nameinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*flistp = NULL;

	int32			elemid;

	void			*vp = NULL;

	void			*valp = NULL;
	int32			perr;
	poid_t			*config_pdp = NULL;
	int32			psm_enable_flag = 0;
	
	poid_t			*a_pdp = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	*out_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);

	/***********************************************************
	 * Walk the input nameinfo array and prep each element.
	 ***********************************************************/
	flistp = PIN_FLIST_ELEM_GET_NEXT(*out_flistpp, PIN_FLD_NAMEINFO,
                &elemid, 1, &cookie, ebufp);
	while (flistp != (pin_flist_t *)NULL) {

		/**************************************************
		 * If country not provided, try to
		 * obtain country name from pin.conf
		 * if not there then use FM_DEFAULT_COUNTRY
		 **************************************************/
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_COUNTRY, 1, ebufp);

		if ((vp == (void *)NULL) || (strlen(vp) == 0)) {

			pin_conf(FM_CUST_POL, FM_COUNTRY_TOKEN, PIN_FLDT_STR,
                        	(caddr_t *)&valp, &perr);

			switch (perr) {
			case PIN_ERR_NONE:
				vp = (void *)valp;
				break;
 
			case PIN_ERR_NOT_FOUND:
				vp = (void *)strdup(FM_DEFAULT_COUNTRY);
				break;
 
			default:
				pin_set_err(ebufp, PIN_ERRLOC_FM,
				  PIN_ERRCLASS_SYSTEM_DETERMINATE, perr, 
				  0, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				  "Unable to read country from pin.conf",ebufp);
				vp = (void *)strdup(FM_DEFAULT_COUNTRY);
				break;
			}
			PIN_FLIST_FLD_PUT(flistp, PIN_FLD_COUNTRY, vp, ebufp);
		} 

		fm_cust_pol_map_country( flistp, flistp, (char *)vp, ebufp);

		/*******************************************************
		 * Payment Suspense Management
		 * Set the account poid in /config/psm if FN and LN match
		 *******************************************************/

		/*********************************************
		 * Check if PSM feature is enabled and POID 
		 * looks like valid /account poid
		 *********************************************/
		psm_enable_flag = psiu_bparams_get_int(ctxp, 
			PSIU_BPARAMS_AR_PARAMS,
			PSIU_BPARAMS_AR_PYMT_SUSPENSE_ENABLE, ebufp);	 
		
		a_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID,
							 0, ebufp);
		
		if ( !PIN_POID_IS_NULL(a_pdp) && 
		psm_enable_flag==PSIU_BPARAMS_AR_PYMT_SUSPENSE_ENABLED &&
		strcmp(PIN_POID_GET_TYPE(a_pdp), "/account") == 0 &&
		PIN_POID_GET_ID(a_pdp) > 0 ){

			fm_cust_pol_prep_nameinfo_psm(ctxp, a_pdp, flistp, ebufp);	

			/*********************************************************
			 * If /config/psm is found set  put it to output
			 *********************************************************/
			config_pdp = PIN_FLIST_FLD_TAKE(flistp,
							PIN_FLD_OBJECT, 1, ebufp);
			if (!PIN_POID_IS_NULL(config_pdp))
				PIN_FLIST_FLD_PUT(*out_flistpp, PIN_FLD_OBJECT,
								config_pdp, ebufp);

		}

		/***************************************************
                 * Get the next nameinfo element.
                 ***************************************************/
                flistp = PIN_FLIST_ELEM_GET_NEXT(*out_flistpp, PIN_FLD_NAMEINFO,
                        &elemid, 1, &cookie, ebufp);
	} 

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_nameinfo error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_map_country()
 *
 *      map country to be WorldTax country code:
 *		set PIN_FLD_CANON_COUNTRY
 *
 *      XXX hard coded list of countries is used XXX
 *
 *******************************************************************/
void
fm_cust_pol_map_country(
	pin_flist_t		*in_flistp,
	pin_flist_t		*r_flistp,
        char            	*country,
        pin_errbuf_t		*ebufp )
{
        char                    *p;
        char                    wt_country[3];
        int                     i;

        static char *Country_list[] = {

	"AF:AF",
	"AF:Afghanistan",
	"AX:AX",
	"AX:Aland Islands",
	"AL:AL",
	"AL:Albania",
	"DZ:DZ",
	"DZ:Algeria",
	"AS:AS",
	"AS:American Samoa",
	"AD:AD",
	"AD:Andorra",
	"AO:AO",
	"AO:Angola",	
	"AI:AI",
	"AI:Anguilla",
	"AQ:AQ",
	"AQ:Antarctica",
	"AG:AG",
	"AG:Antigua and Barbuda",
        "AR:AR",
        "AR:Argentina",
	"AM:AM",
	"AM:Armenia",
	"AW:AW",
	"AW:Aruba",
        "AU:AU",
        "AU:Australia",
        "AT:AT",
        "AT:Austria",
	"AZ:AZ",
	"AZ:Azerbaijan",
	"BS:BS",
	"BS:Bahamas",
	"BH:BH",
	"BH:Bahrain",
	"BD:BD",
	"BD:Bangladesh",
	"BB:BB",
	"BB:Barbados",
	"BY:BY",
	"BY:Belarus",
	"BE:BE",
	"BE:Belgium",
	"BZ:BZ",
	"BZ:Belize",
	"BJ:BJ",
	"BJ:Benin",
	"BM:BM",
	"BM:Bermuda",
	"BT:BT",
	"BT:Bhutan",
	"BO:BO",
	"BO:Bolivia",
	"BQ:BQ",		
	"BQ:Bonaire, Sint Eustatius and Saba",
	"BA:BA",
	"BA:Bosnia-Herzegovina",
	"BW:BW",
	"BW:Botswana",
	"BV:BV",
	"BV:Bouvet Island",
	"BR:BR",
	"BR:Brazil",
	"IO:IO",
	"IO:British Indian Ocean Territory",
	"VG:VG",
	"VG:British Virgin Islands",
	"BN:BN",
	"BN:Brunei",
	"BG:BG",
	"BG:Bulgaria",
	"BF:BF",
	"BF:Burkina Faso",
	"BI:BI",
	"BI:Burundi",
	"KH:KH",
	"KH:Cambodia",
	"KH:Kampuchea",
	"CM:CM",
	"CM:Cameroon",
        "CA:CA",
        "CA:Canada",
	"CV:CV",
	"CV:Cape Verde",
	"KY:KY",
	"KY:Cayman Islands",
	"CF:CF",
	"CF:Central African Republic",
	"TD:TD",
	"TD:Chad",
	"CL:CL",
	"CL:Chile",
	"CN:CN",
	"CN:China",
	"CN:People's Republic of China",
	"CX:CX",
	"CX:Christmas Island",
	"CC:CC",
	"CC:Cocos Islands",
	"CC:Keeling Islands",
	"CO:CO",
	"CO:Colombia",
	"KM:KM",
	"KM:Comoros",
	"CG:CG",
	"CG:Congo",
	"CD:CD",
	"CD:Democratic Republic of Congo",
	"CK:CK",
	"CK:Cook Islands",
	"CR:CR",
	"CR:Costa Rica",
	"CI:CI",
	"CI:Cote D'Ivoire",
	"HR:HR",
	"HR:Croatia",
	"CU:CU",
	"CU:Cuba",
	"CW:CW",
	"CW:Curacao",
	"CY:CY",
	"CY:Cyprus",
	"CZ:CZ",
	"CZ:Czech Republic",
	"DK:DK",
	"DK:Denmark",
	"DJ:DJ",
	"DJ:Djibouti",
	"DM:DM",
	"DM:Dominica",
	"DO:DO",
	"DO:Dominican Republic",
	"TP:TP",
	"TP:East Timor",
	"EC:EC",
	"EC:Ecuador",
	"EG:EG",
	"EG:Egypt",
	"SV:SV",
	"SV:El Salvador",
	"GQ:GQ",
	"GQ:Equatorial Guinea",
	"ER:ER",
	"ER:Eritrea",
	"EE:EE",
	"EE:Estonia",
	"ET:ET",
	"ET:Ethiopia",
	"FK:FK",
	"FK:Falkland Islands",
	"FK:Malvinas",
	"FO:FO",
	"FO:Faroe Island",
	"FJ:FJ",
	"FJ:Fiji",
	"FI:FI",
	"FI:Finland",
	"FR:FR",
	"FR:France",
	"FX:FX",
	"FX:Metropolitan France",
	"GF:GF",
	"GF:French Guiana",
	"PF:PF",
	"PF:French Polynesia",
	"TF:TF",
	"TF:French Southern Territories",
	"GA:GA",
	"GA:Gabon",
	"GM:GM",
	"GM:Gambia",
	"GE:GE",
	"GE:Georgia",
	"DE:DE",
	"DE:Germany",
	"GH:GH",
	"GH:Ghana",
	"GI:GI",
	"GI:Gibraltar",
        "GB:GB",
        "GB:Great Britain",
        "GB:England",
        "GB:United Kingdom",
	"GR:GR",
	"GR:Greece",
        "EL:EL",
        "EL:Greece",
	"GL:GL",
	"GL:Greenland",
	"GD:GD",
	"GD:Grenada",
	"GP:GP",
	"GP:Guadeloupe",
	"GU:GU",
	"GU:Guam",
	"GT:GT",
	"GT:Guatemala",
	"GG:GG",
	"GG:Guernsey",
	"GN:GN",
	"GN:Guinea",
	"GW:GW",
	"GW:Guinea Bissau",
	"GY:GY",
	"GY:Guyana",
	"HT:HT",
	"HT:Haiti",
	"HM:HM",
	"HM:Heard and McDonald Islands",
	"VA:VA",
	"VA:Vatican",
	"VA:Holy See",
	"HN:HN",
	"HN:Honduras",
	"HK:HK",
	"HK:Hong Kong",
	"HU:HU",
	"HU:Hungary",
	"IS:IS",
	"IS:Iceland",
	"IN:IN",
	"IN:India",
	"ID:ID",
	"ID:Indonesia",
	"IR:IR",
	"IR:Iran",
	"IQ:IQ",
	"IQ:Iraq",
	"IE:IE",
	"IE:Ireland",
	"IM:IM",
	"IM:Isle of man",
	"IL:IL",
	"IL:Israel",
	"IT:IT",
	"IT:Italy",
	"JM:JM",
	"JM:Jamaica",
	"JP:JP",
	"JP:Japan",
	"JE:JE",
	"JE:Jersey",
	"JO:JO",
	"JO:Jordan",
	"KZ:KZ",
	"KZ:Kazakhstan",
	"KE:KE",
	"KE:Kenya",
	"KI:KI",
	"KI:Kiribati",
	"KP:KP",
	"KP:People's Republic of Korea",
	"KP:North Korea",
	"KR:KR",
	"KR:Republic of Korea",
	"KR:South Korea",
	"KW:KW",
	"KW:Kuwait",
	"KG:KG",
	"KG:Kyrgyzstan",
	"LA:LA",
	"LA:Lao People's Democratic Republic",
	"LV:LV",
	"LV:Latvia",
	"LB:LB",
	"LB:Lebanon",
	"LS:LS",
	"LS:Lesotho",
	"LR:LR",
	"LR:Liberia",
	"LY:LY",
	"LY:Libyan Arab Jamahiriya",
	"LI:LI",
	"LI:Liechtenstein",
	"LT:LT",
	"LT:Lithuania",
	"LU:LU",
	"LU:Luxembourg",
	"LU:Luxemborg",
	"MO:MO",
	"MO:Macao",
	"MK:MK",
	"MK:Macedonia",
	"MG:MG",
	"MG:Madagascar",
	"MW:MW",
	"MW:Malawi",
	"MY:MY",
	"MY:Malaysia",
	"MV:MV",
	"MV:Maldives",
	"ML:ML",
	"ML:Mali",
	"MT:MT",
	"MT:Malta",
	"MH:MH",
	"MH:Marshall Islands",
	"MQ:MQ",
	"MQ:Martinique",
	"MR:MR",
	"MR:Mauritania",
	"MU:MU",
	"MU:Mauritius",
	"YT:YT",
	"YT:Mayotte",
	"MX:MX",
	"MX:Mexico",
	"FM:FM",
	"FM:Micronesia",
	"MD:MD",
	"MD:Moldova",
	"MD:Republic of Moldova",
	"MC:MC",
	"MC:Monaco",
	"MN:MN",
	"MN:Mongolia",
	"ME:ME",
	"ME:Montenegro",
	"MS:MS",
	"MS:Montserrat",
	"MA:MA",
	"MA:Morocco",
	"MZ:MZ",
	"MZ:Mozambique",
	"MM:MM",
	"MM:Myanmar",
	"NA:NA",
	"NA:Namibia",
	"NR:NR",
	"NR:Nauru",
	"NP:NP",
	"NP:Nepal",
	"NL:NL",
        "NL:Netherlands",
        "NL:Holland",
	"AN:AN",
	"AN:Netherlands Antilles",
	"NC:NC",
	"NC:New Caledonia",
	"NC:New Caledonia and Dependencies",
	"NZ:NZ",
	"NZ:New Zealand",
	"NI:NI",
	"NI:Nicaragua",
	"NE:NE",
	"NE:Niger",
	"NG:NG",
	"NG:Nigeria",
	"NU:NU",
	"NU:Niue",
	"NF:NF",
	"NF:Norfolk Islands",
	"MP:MP",
	"MP:Northern Mariana Islands",
        "NO:NO",
        "NO:Norway",
	"OM:OM",
	"OM:Oman",
	"PK:PK",
	"PK:Pakistan",
	"PW:PW",
	"PW:Palau",
	"PS:PS",
	"PS:Palestine, State of",
	"PA:PA",
	"PA:Panama",
	"PG:PG",
	"PG:Papua New Guinea",
	"PY:PY",
	"PY:Paraguay",
	"PE:PE",
	"PE:Peru",
	"PH:PH",
	"PH:Philippines",
	"PN:PN",
	"PN:Pitcairn",
	"PL:PL",
	"PL:Poland",
	"PT:PT",
	"PT:Portugal",
	"PR:PR",
	"PR:Puerto Rico",
	"QA:QA",
	"QA:Qatar",
	"RE:RE",
	"RE:Reunion",
	"RO:RO",
	"RO:Romania",
	"RU:RU",
	"RU:Russian Federation",
	"RW:RW",
	"RW:Rwanda",
	"BL:BL",
	"BL:Saint Barthelemy",
	"GS:GS",
	"GS:South Georgia and South Sandwich",
	"GS:South Georgia",
	"GS:South Sandwich",
	"SS:SS",
	"SS:South Sudan",
	"WS:WS",
	"WS:Samoa",
	"SM:SM",
	"SM:San Marino",
	"ST:ST",
	"ST:Sao Tome and Principe",
	"SA:SA",
	"SA:Saudi Arabia",
	"SN:SN",
	"SN:Senegal",
	"RS:RS",
	"RS:Serbia",
	"SC:SC",
	"SC:Seychelles",
	"SL:SL",
	"SL:Sierra Leone",
	"SG:SG",
	"SG:Singapore",
	"SX:SX",
	"SX:Sint Maarten (Dutch Part)",
	"SK:SK",
	"SK:Slovakia",
	"SI:SI",
	"SI:Slovenia",
	"SB:SB",
	"SB:Solomon Islands",
	"SO:SO",
	"SO:Somalia",
	"ZA:ZA",
	"ZA:South Africa",
        "ES:ES",
        "ES:Spain",
	"LK:LK",
	"LK:Sri Lanka",
	"SH:SH",
	"SH:St. Helena",
	"SH:Saint Helena",
	"SH:St. Helena and Dependencies",
	"KN:KN",
	"KN:Saint Kitts",
	"KN:St. Kitts",
	"KN:Nevis",
	"KN:St. Kitts and Nevis",
	"LC:LC",
	"LC:St. Lucia",
	"LC:Saint Lucia",
	"MF:MF",
	"MF:Saint Martin (french part)",
	"PM:PM",
	"PM:St. Pierre and Miquelon",
	"PM:St. Pierre",
	"PM:Saint Pierre",
	"PM:Miquelon",
	"VC:VC",
	"VC:St. Vincent and Grenadine",
	"VC:St. Vincent",
	"VC:Saint Vincent",
	"VC:Grenadine",
	"SD:SD",
	"SD:Sudan",
	"SR:SR",
	"SR:Suriname",
	"SJ:SJ",
	"SJ:Svalbard and Jan Mayen",
	"SZ:SZ",
	"SZ:Swaziland",
        "SE:SE",
        "SE:Sweden",
        "CH:CH",
        "CH:Switzerland",
	"SY:SY",
	"SY:Syrian Arab Republic",
        "TW:TW",
        "TW:Taiwan",
	"TW:Taiwan ROC",
	"TW:Taiwan Republic of China",
	"TJ:TJ",
	"TJ:Tajikistan",
	"TZ:TZ",
	"TZ:Tanzania",
        "TH:TH",
        "TH:Thailand",
	"TL:TL",
	"TL:Timor-Leste",
	"TG:TG",
	"TG:Togo",
	"TK:TK",
	"TK:Tokelau",
	"TO:TO",
	"TO:Tonga",
	"TT:TT",
	"TT:Trinidad and Tobago",
	"TT:Trinidad",
	"TT:Tobago",
	"TN:TN",
	"TN:Tunisia",
	"TR:TR",
	"TR:Turkey",
	"TM:TM",
	"TM:Turkmenistan",
	"TC:TC",
	"TC:Turks and Caicos Islands",
	"TV:TV",
	"TV:Tuvalu",
	"UG:UG",
	"UG:Uganda",
	"UA:UA",
	"UA:Ukraine",
	"AE:AE",
	"AE:United Arab Emirates",
        "US:US",
        "US:USA",
        "US:United States of America",
	"UY:UY",
	"UY:Uruguay",
	"UM:UM",
	"UM:U.S. Minor Outlying Islands",
	"UZ:UZ",
	"UZ:Uzbekistan",
	"VU:VU",
	"VU:Vanuatu",
	"VE:VE",
	"VE:Venezuela",
	"VN:VN",
	"VN:Vietnam",
	"VN:Viet Nam",
	"VI:VI",
	"VI:Virgin Islands",
	"WF:WF",
	"WF:Wallis and Futuna Islands",
	"EH:EH",
	"EH:Western Sahara",
	"YE:YE",
	"YE:Yemen",
	"YU:YU",
	"YU:Yugoslavia",
	"ZM:ZM",
	"ZM:Zambia",
	"ZW:ZW",
	"ZW:Zimbabwe",	
        ""     
        };


        /*
         * Walk through the list ...
         */
        for( i = 0; (p = strchr( Country_list[i], ':')); i++ )  {
                if( country && !strcasecmp(country, p+1)) {
                        memcpy( wt_country, Country_list[i], 2);
                        wt_country[2] = '\0';
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_CANON_COUNTRY, 
						(void *)wt_country, ebufp);
                        return;
                        /*****/
                }
        }

        /*
         * Unknown country
	 * 
         */    
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_CANON_COUNTRY, (void *)NULL, ebufp);

        return;
}

/*************************************************************
 * Payment Suspense Management
 * Set the account poid in /config/psm if First Name and
 * Last Name starts with "PAYMENT" and "SUSPENSE" respectively
 *************************************************************/
static void
fm_cust_pol_prep_nameinfo_psm(
	pcm_context_t	*ctxp,
	poid_t		*a_pdp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*a_flistp = NULL;
	poid_t		*pdp = NULL;
	poid_t		*i_pdp = NULL;
	poid_t		*old_a_pdp = NULL;
	void		*vp = NULL;
	char*		fname = NULL;
	char*		lname = NULL;
	char*		templ = NULL;
	int32		flags = 0;
	int32		psm_enable_flag = 0;
	int32		elemid = 0;
        int64           primary_db_no = 0;
        int64           ctx_db_no = 0;
	u_int		create = 1;
	pin_cookie_t	cookie = NULL;
        pcm_context_t   *vctxp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return ;
	PIN_ERR_CLEAR_ERR(ebufp);	

	fname = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FIRST_NAME, 1, ebufp);
	lname = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LAST_NAME, 1, ebufp); 

	if( fname && lname &&
		(strncasecmp(fname, PIN_SUSPENSE_FNAME, sizeof(PIN_SUSPENSE_FNAME)-1) == 0) &&
		(strncasecmp(lname, PIN_SUSPENSE_LNAME, sizeof(PIN_SUSPENSE_LNAME)-1) == 0)){
		
		/*********************************************************
		 * Look for /config/psm object, expecting only one object
		 * create one if not present
		 *********************************************************/
		s_flistp = PIN_FLIST_CREATE(ebufp);

                ctx_db_no = cm_fm_get_current_db_no(ctxp);
                if(cm_fm_is_multi_db()) {
                        cm_fm_get_primary_db_no(&primary_db_no, ebufp);
                }
                else {
                        primary_db_no = ctx_db_no;
		}

		/*********************************************************
		 * We should always search for config objects in primary
		 * db and also create the object in primary only since 
		 * config table is a view in secondary
                 *********************************************************/
		 
		vp = PIN_POID_CREATE(primary_db_no, "/search", -1, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)vp, ebufp);
		
		templ = "select X from /config where F1 = V1 ";
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE,
						(void *)templ, ebufp); 
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, 
						(void *)&flags, ebufp);		 
		pdp = PIN_POID_CREATE(primary_db_no, "/config/psm", -1, ebufp);
		a_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_PUT(a_flistp, PIN_FLD_POID, (void *)pdp, ebufp);				 
		
		/*********************************************************
		 * Return everything
		 *********************************************************/
		PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

                /*********************************************************
		 * Change the context to primary db if we are in secondary 
                 *********************************************************/

		if ( primary_db_no != ctx_db_no ) {
			PCM_CONTEXT_OPEN(&vctxp, (pin_flist_t *)0, ebufp);
		} else {
			vctxp = ctxp;
		}
				
		PCM_OP(vctxp, PCM_OP_SEARCH, PCM_OPFLG_READ_UNCOMMITTED, 
						s_flistp, &r_flistp, ebufp);

                /***************************************************
                * Close the previously opened context.
                 ***************************************************/

                if ( primary_db_no != ctx_db_no ) {
                        PCM_CONTEXT_CLOSE(vctxp, 0, ebufp);
                }
		
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		
		/*********************************************************
		 * Fetch Poid from output
		 *********************************************************/
		a_flistp = PIN_FLIST_ELEM_GET(r_flistp,
					PIN_FLD_RESULTS, 0, 1, ebufp);
		if (a_flistp){
			i_pdp = PIN_FLIST_FLD_TAKE(a_flistp, PIN_FLD_POID, 1, ebufp);
			/****************************************************
			 * Check if we are updating the account specified in
			 * /config/psm
			 ****************************************************/
			while ((flistp = PIN_FLIST_ELEM_GET_NEXT(a_flistp,
				PIN_FLD_ACCOUNTS, &elemid, 1, &cookie, ebufp)) !=
							(pin_flist_t *)NULL) {

				old_a_pdp = PIN_FLIST_FLD_GET(flistp,
						PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
							
				if (PIN_POID_COMPARE(old_a_pdp, a_pdp, 0, ebufp) == 0)
				{
					create = 0;
					break;
				}
			}
		}
		/*********************************************************
		 * Set to create or Update /confg/psm if account poid is 
		 * not already in /config/psm
		 *********************************************************/
		if (create){
			if (PIN_POID_IS_NULL(i_pdp)){
				vp = PIN_POID_CREATE(primary_db_no,
						"/config/psm", -1, ebufp);
				PIN_FLIST_FLD_PUT(i_flistp,
						PIN_FLD_OBJECT, vp , ebufp);
			} else {
				PIN_FLIST_FLD_PUT(i_flistp,
						PIN_FLD_OBJECT, i_pdp, ebufp);
			}
		}
		
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	}	

	if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_nameinfo_psm error", ebufp);
	}
	return;
}
