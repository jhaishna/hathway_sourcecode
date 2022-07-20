/*******************************************************************
 *
* Copyright (c) 1999, 2017, Oracle and/or its affiliates. All rights reserved.
 * 
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@#$Id: fm_cust_pol_canonicalize.c /cgbubrm_mainbrm.portalbase/1 2017/07/27 11:39:36 sagaur Exp $"; 
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"

#define DEFAULT_LOCALE	"en_US"

EXPORT_OP void
op_cust_pol_canonicalize(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_canonicalize(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);
 
static int
fm_cust_pol_latinutf8_to_canonical(
	unsigned char 		*in_string, 
	unsigned char		*out_string);

static int 
fm_cust_pol_getutf8canon(
	unsigned char 		*cp1, 
	unsigned char 		*cp2);

static char * 
fm_cust_pol_convert_locale(
	char *inlocalep);

struct locale_to_lcid {
	char *suplocalesp;
	char *portallocalep;
} locale_to_lcid[] = {
	{ "AFK",        "AFK"   },              /* Afrikaans */
	{ "ca",         "CA"    },              /* Catalan */
	{ "CAT",        "CA"    },              /* Catalan */
	{ "da",         "DAN"   },              /* Danish  */
	{ "DAN",        "DAN"   },
	{ "de",		"DE"	},		/* Germany */
	{ "DEC",        "DEC"   },              /* Liechenstein */
	{ "DEU",	"DE"	},
	{ "de_AT",	"DE_AT"	},		/* Austria */
	{ "DEA",	"DE_AT"	},
	{ "de_CH",	"DE_CH"	},		/* Czech   */
	{ "DES",	"DE_CH"	},
	{ "en_US",	"EN_US" },		/* USA	   */
	{ "ENU",	"EN_US" },
	{ "English (United States) :ENU",	"EN_US" },
	{ "English (United States)",		"EN_US" },
	{ "en_AU",	"EN_AU" },		/* Australia */
	{ "ENA",	"EN_AU" },
	{ "ENB",        "ENB"   },              /* Caribbean */
	{ "en_CA",	"EN_CA" },		/* Canada  */
	{ "ENC",	"EN_CA" },
	{ "en_NZ",	"EN_NZ" },		/* New Zealand */
	{ "ENZ",	"EN_NZ" },
	{ "en_UK",	"EN_UK" },		/* United Kingdom */
	{ "ENG",	"EN_UK" },
	{ "en_IE",	"EN_IE" },		/* Ireland */
	{ "ENI",	"EN_IE" },		/* Ireland English*/
	{ "ENJ",        "ENJ"   },              /* Jamaica */
	{ "ENL",        "ENL"   },              /* Belize  */
	{ "ENS",        "ENS"   },              /* South Africa  */
	{ "ENT",        "ENT"   },              /* Trinidad  */
	{ "IRL",	"EN_IE" },
	{ "es",		"ES"	},		/* Spain   */
	{ "ESP",	"ES"	},
	{ "ESS",        "ES_AR" },              /* Argentina */
	{ "es_AR",      "ES_AR" },
	{ "ESB",        "ES_BO" },              /* Bolivia */
	{ "es_BO",      "ES_BO" },
	{ "ESL",        "ES_CL" },              /* Chile */
	{ "es_CL",      "ES_CL" },
	{ "ESO",        "ES_CO" },              /* Colombia */
	{ "es_CO",      "ES_CO" },
	{ "ESC",        "ES_CR" },              /* Costa Rica */
	{ "es_CR",      "ES_CR" },
	{ "ESD",        "ESD"   },              /* Dominican Republic */
	{ "ESF",        "ES_EC" },              /* Ecuador    */
	{ "es_EC",      "ES_EC" },
	{ "ESE",        "ES_SV" },              /* El Salvador */
	{ "es_SV",      "ES_SV" },
	{ "ESG",        "ES_GT" },              /* Guatemala */
	{ "es_GT",      "ES_GT" },
	{ "ESH",        "ESH"   },              /* Honduras */
	{ "ESN",        "ESN"   },              /* Modern  */
	{ "es_NI",      "ES_NI" },              /* Nicaragua  */
	{ "ESI",        "ES_NI" },
	{ "es_PA",      "ES_PA" },              /* Panama  */
	{ "ESA",        "ES_PA" },
	{ "es_PY",      "ES_PY" },              /* Paraguay  */
	{ "ESZ",        "ES_PY" },
	{ "es_PE",      "ES_PE" },              /* Peru  */
	{ "ESR",        "ES_PE" },
	{ "ESU",        "ESU"   },              /* Puerto Rico */
	{ "es_UY",      "ES_UY" },              /* Uruguay  */
	{ "ESY",        "ES_UY" },
	{ "EU",         "EU"    },              /* Belize  */
	{ "EUQ",        "EU"    },              /* Belize  */
	{ "FOS",        "FOS"   },              /* Faeroese  */
	{ "es_MX",	"ES_MX"	},		/* Mexico  */
	{ "ESM",	"ES_MX"	},
	{ "fr",		"FR"	},		/* France  */
	{ "FRA",	"FR"	},
	{ "fr_BE",	"FR_BE"	},		/* Belgium */
	{ "FRB",	"FR_BE"	},
	{ "fr_CA",	"FR_CA"	},		/* French Canada */
	{ "FRC",	"FR_CA"	},
	{ "fr_CH",	"FR_CH"	},		/* French Czech */
	{ "FRS",	"FR_CH"	},
	{ "IND",        "IND"   },              /* Indonesia */
	{ "it",		"IT"	},		/* Italy   */
	{ "ITA",	"IT"	},
	{ "LVI",        "LVI"   },              /* Latvia  */
	{ "LTH",        "LTH"   },              /* Lithuania  */
	{ "nl",		"NL"	},
	{ "NLD",	"NL"	},		/* Netherlands */
	{ "nl_BE",	"NL_BE"	},
	{ "NLB",	"NL_BE"	},
	{ "NOR",        "NOR"   },              /* Norwegian   */
	{ "NON",        "NON"   },              /* Norwegian   */
	{ "pt",		"PT"	},		/* Portugal */
	{ "PRT",	"PT"	},
	{ "PTB",        "PT"    },
	{ "PTG",	"PT"	},
	{ "su",		"FI"	},		/* Belgium  */
	{ "FIN",	"FI"	},
	{ "fi",         "FI"    },              /* Finland  */
	{ "sv",         "SV"    },              /* Sweden   */
	{ "SVE",        "SV"    },              /* Sweden   */
	{ "SVF",        "SV"    },              /* Sweden   */
	{ "FRL",	"FRL"	},
	{ "DEL",	"DE"	},
	{ "ITS",	"ITS"	},
}; 


/***********************************************************
 *
 * trans_UTF8_canon table is a decimal number table for UTF8 
 * characters.This table maps UTF8 ANSII characters to UTF8 
 * Canonical characters.
 * First half (0-255) of the table presents 8bit ASCII in 
 * UTF8 values and
 * Second half (256-511) presents equivalent Canonical 
 * values of that character. 
 *
 **********************************************************/

static int32
trans_UTF8_canon[] = {
/*   ASCII  in UTF8 here */
/*   0 -   4 */		0, 1, 2, 3, 4, 
/*   5 -   9 */		5, 6, 7, 8, 9,
/*  10 -  14 */		10, 11, 12, 13, 14, 
/*  15 -  19 */		15, 16, 17, 18, 19,
/*  20 -  24 */		20, 21, 22, 23, 24,
/*  25 -  29 */		25, 26, 27, 28, 29,
/*  30 -  34 */		30, 31, 32, 33, 34,
/*  35 -  39 */		35, 36, 37, 38, 39,
/*  40 -  44 */		40, 41, 42, 43, 44,
/*  45 -  49 */		45, 46, 47, 48, 49,
/*  50 -  54 */		50, 51, 52, 53, 54,
/*  55 -  59 */		55, 56, 57, 58, 59,
/*  60 -  64 */		60, 61, 62, 63, 64,
/*  65 -  69 */		65, 66, 67, 68, 69,
/*  70 -  74 */		70, 71, 72, 73, 74,
/*  75 -  79 */		75, 76, 77, 78, 79,
/*  80 -  84 */		80, 81, 82, 83, 84,
/*  85 -  89 */		85, 86, 87, 88, 89,
/*  90 -  94 */		90, 91, 92, 93, 94,
/*  95 -  99 */		95, 96, 97, 98, 99,
/* 100 - 104 */		100, 101, 102, 103, 104,
/* 105 - 109 */		105, 106, 107, 108, 109,
/* 110 - 114 */		110, 111, 112, 113, 114,
/* 115 - 119 */		115, 116, 117, 118, 119,
/* 120 - 129 */		120, 121, 122, 123, 124,
/* 125 - 129 */		125, 126, 127, 49792, 49793,
/* 130 - 134 */		14844058, 50834, 14844062, 14844070, 14844064,
/* 135 - 139 */		14844065, 52102, 14844080, 50592, 14844089,
/* 140 - 144 */		50578, 49805, 49806, 49807, 49808,
/* 145 - 149 */		14844056, 14844057, 14844060, 14844061, 14844066,
/* 150 - 154 */		14844051, 14844052, 52124, 14845090, 50593,
/* 155 - 159 */		14844090, 50579, 49821, 49822, 50616,
/* 160 - 164 */		49824, 49825, 49826, 49827, 49828,
/* 165 - 169 */		49829, 49830, 49831, 49832, 49833,
/* 170 - 174 */		49834, 49835, 49836, 49837, 49838,
/* 175 - 179 */		49839, 49840, 49841, 49842, 49843,
/* 180 - 184 */		49844, 49845, 49846, 49847, 49848,
/* 185 - 189 */		49849, 49850, 49851, 49852, 49853,
/* 190 - 194 */		49854, 49855, 50048, 50049, 50050,
/* 195 - 199 */		50051, 50052, 50053, 50054, 50055,
/* 200 - 204 */		50056, 50057, 50058, 50059, 50060,
/* 205 - 209 */		50061, 50062, 50063, 50064, 50065,
/* 210 - 214 */		50066, 50067, 50068, 50069, 50070,
/* 215 - 219 */		50071, 50072, 50073, 50074, 50075,
/* 220 - 224 */		50076, 50077, 50078, 50079, 50080,
/* 225 - 229 */		50081, 50082, 50083, 50084, 50085,
/* 230 - 234 */		50086, 50087, 50088, 50089, 50090,
/* 235 - 239 */		50091, 50092, 50093, 50094, 50095,
/* 240 - 244 */		50096, 50097, 50098, 50099, 50100,
/* 245 - 249 */		50101, 50102, 50103, 50104, 50105,
/* 250 - 255 */		50106, 50107, 50108, 50109, 50110, 50111,

/* ASCII equivalent Canonical form in UTF8 here */
/*   0 -   4 */		0, 1, 2, 3, 4, 
/*   5 -   9 */		5, 6, 7, 8, 9,
/*  10 -  14 */		10, 11, 12, 13, 14, 
/*  15 -  19 */		15, 16, 17, 18, 19,
/*  20 -  24 */		20, 21, 22, 23, 24,
/*  25 -  29 */		25, 26, 27, 28, 29,
/*  30 -  34 */		30, 31, 32, 33, 34,
/*  35 -  39 */		35, 36, 37, 38, 39,
/*  40 -  44 */		40, 41, 42, 43, 44,
/*  45 -  49 */		45, 46, 47, 48, 49,
/*  50 -  54 */		50, 51, 52, 53, 54,
/*  55 -  59 */		55, 56, 57, 58, 59,
/*  60 -  64 */		60, 61, 62, 63, 64,
/*  65 -  69 */		97, 98, 99, 100, 101,
/*  70 -  74 */		102, 103, 104, 105, 106,
/*  75 -  79 */		107, 108, 109, 110, 111,
/*  80 -  84 */		112, 113, 114, 115, 116,
/*  85 -  89 */		117, 118, 119, 120, 121,
/*  90 -  94 */		122, 91, 92, 93, 94,
/*  95 -  99 */		95, 96, 97, 98, 99,
/* 100 - 104 */		100, 101, 102, 103, 104,
/* 105 - 109 */		105, 106, 107, 108, 109,
/* 110 - 114 */		110, 111, 112, 113, 114,
/* 115 - 119 */		115, 116, 117, 118, 119,
/* 120 - 124 */		120, 121, 122, 123, 124,
/* 125 - 129 */		125, 126, 127, 49792, 49793,
/* 130 - 134 */		14844058, 50834, 14844062, 14844070, 14844064,
/* 135 - 139 */		14844065, 52102, 14844080, 115, 14844089,
/* 140 - 144 */		111, 49805, 49806, 49807, 49808,
/* 145 - 149 */		14844056, 14844057, 14844060, 14844061, 14844066,
/* 150 - 154 */		14844051, 14844052, 52124, 14845090, 50593,
/* 155 - 159 */		14844090, 111, 49821, 49822, 121,
/* 160 - 164 */		49824, 49825, 49826, 49827, 49828,
/* 165 - 169 */		121, 49830, 49831, 49832, 49833,
/* 170 - 174 */		49834, 49835, 49836, 49837, 49838,
/* 175 - 179 */		49839, 49840, 49841, 49842, 49843,
/* 180 - 184 */		49844, 49845, 49846, 49847, 49848,
/* 185 - 189 */		49849, 49850, 49851, 49852, 49853,
/* 190 - 194 */		49854, 49855, 97, 97, 97,
/* 195 - 199 */		97, 97, 97, 97, 99,
/* 200 - 204 */		101, 101, 101, 101, 105,
/* 205 - 209 */		105, 105, 105, 100, 110,
/* 210 - 214 */		111, 111, 111, 111, 111,
/* 215 - 219 */		50071, 111, 117, 117, 117,
/* 220 - 224 */		117, 121, 50078, 50079, 97,
/* 225 - 229 */		97, 97, 97, 97, 97,
/* 230 - 234 */		97, 99, 101, 101, 101,
/* 235 - 239 */		101, 105, 105, 105, 105,
/* 240 - 244 */		111, 110, 111, 111, 111,
/* 245 - 249 */		111, 111, 50103, 111, 117,
/* 250 - 255 */		117, 117, 117, 121, 50110, 121
	};

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_CANONICALIZE  command
 *******************************************************************/
void
op_cust_pol_canonicalize(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_CANONICALIZE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_canonicalize", ebufp);
		return;
	}

	/******************************************************************
	 * Debug: What we got.
	 ******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_canonicalize input flist", in_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_canonicalize(ctxp, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_canonicalize error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_canonicalize return flist", r_flistp);
	}

	return;
}


/*******************************************************************
 * fm_cust_pol_canonicalize()
 *
 * The actual routine which does the canonicalization of the string	
 *
 *******************************************************************/
static void
fm_cust_pol_canonicalize(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*str_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	char			*localep = NULL;
	unsigned char		*instringp = NULL;
	unsigned char		*outstringp = NULL;
	void			*vp = NULL;
	char			locale[BUFSIZ];

	int32   		ret_val = -1;
	int32			err = 0;
	int32			rec_id = 0;
	u_int32			result = PIN_ERR_NONE;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Do we have the locale information? 
	 ***********************************************************/
	localep = (char *)PIN_FLIST_FLD_GET(in_flistp, 
			PIN_FLD_LOCALE, 1, ebufp);
	if ((localep == (void *)NULL) || (strlen(localep) == 0)) {

		pin_conf("fm_cust_pol", "locale", PIN_FLDT_STR, 
			&localep, &err);
		if (localep == NULL) {
			strcpy(locale, DEFAULT_LOCALE);
		} else {
			pin_strlcpy(locale, localep, sizeof(locale));
			free(localep);
		}	
	} else {
		strcpy(locale, localep);
	}

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	*out_flistpp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, vp, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_LOCALE, locale, ebufp);

	/***********************************************************
	 * Walk the PIN_FLD_STR_VALS and canonicalize the strings.
	 ***********************************************************/
	while ((str_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, 
		PIN_FLD_STR_VALS, &rec_id, 1, &cookie, 
		ebufp)) != (pin_flist_t *)NULL) {

		flistp = PIN_FLIST_ELEM_ADD(*out_flistpp, 
				PIN_FLD_STR_VALS, rec_id, ebufp); 
		instringp = (unsigned char *)PIN_FLIST_FLD_GET(str_flistp,
				PIN_FLD_STR_VAL, 0, ebufp);

        	if ((instringp == (unsigned char *)NULL) ||
                	(strlen((char *)instringp) == 0)) {

			result = PIN_ERR_NONE;
		} else {
			outstringp = (unsigned char *)pin_malloc(
				(strlen ((char*)instringp) + 1) * 
				sizeof(unsigned char) * 5);

			if (outstringp == (unsigned char *)NULL) {
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NO_MEM, 0,0,0);
				return;
                	}

			if (fm_cust_pol_convert_locale(locale) != NULL) { 
				ret_val = fm_cust_pol_latinutf8_to_canonical(
					instringp, outstringp);
				if (ret_val == -1) {
					result = PIN_ERR_CANON_CONV;
				}
			} else {
				result = PIN_ERR_UNSUPPORTED_LOCALE;
			}
		}

		if (result == PIN_ERR_CANON_CONV) {
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, 
				(void *)&result, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_STR_VAL, 
				instringp, ebufp);
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERR_CANON_CONV, 
				result, 0, 0, 0);
		} else if (result == PIN_ERR_UNSUPPORTED_LOCALE) {
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT,
				(void *)&result, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_STR_VAL,
				instringp, ebufp);
		} else {      /* no errors in conversion */
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT,
				(void *)&result, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_STR_VAL,
				outstringp, ebufp);
		}
	}
 
	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_canonicalize error", ebufp);
	}

	if ( outstringp ) {
		pin_free(outstringp);
	}
	return;
}

/****************************************************************
 * fm_cust_pol_latinutf8_to_canonical()
 *
 *	'Canonicalize' a string by stripping leading and
 *	trailing whitespace, squishing multiple whitespaces
 *	into a single space,
 *	and converting all characters to lower case.
 *
 *	unsigned char *in_string	UTF8 input string 
 *	unsigned char *out_string	canon output string 
 *
 ****************************************************************/

static int
fm_cust_pol_latinutf8_to_canonical(
	unsigned char 		*in_stringp, 
	unsigned char		*out_stringp)
{
	int 		i;
	int		ret=1;
	unsigned char	*cp1 = NULL; 
	unsigned char	*cp2 = NULL; 
	int		space_found = 0;

	if ((in_stringp == (unsigned char *)NULL) || 
		(out_stringp == (unsigned char *)NULL) ||
		(strlen((char *)in_stringp) == 0)) {
			return(-1);
	}

	/***********************************************************
	 * Strip leading spaces.
	 ***********************************************************/
	cp2 = (unsigned char *)in_stringp;
	while (*cp2 && isspace(*cp2 & 0xFF)) {
		cp2++;
	}

	/***********************************************************
	 * Strip trailing spaces.
	 ***********************************************************/
	i = strlen((const char *)in_stringp);
	cp1 = in_stringp + i - 1;

	while (isspace(*cp1 & 0xFF) && (cp1 > cp2)) {
		*cp1 = '\0';
		cp1--;
	}

	/***********************************************************
	 * Start searching for spaces and filtering
	 ***********************************************************/
	cp1 = (unsigned char *)out_stringp;

	while (*cp2 != '\0') {
		if (isspace(*cp2 & 0xFF)) {
			*cp1 = ' ';
			while (isspace(*cp2 & 0xFF)) {
				cp2++;
				space_found = 1;
			}
			ret=1;
		} else {
			ret = fm_cust_pol_getutf8canon(cp1, cp2);
			if (ret == 0) {
				return (-1);
			}
		}

		cp1+=ret;
		if (!space_found) {
			if (((*cp2 & 0xff) & 0x80) == 0x00) { 
				cp2+=1;
			} else if (((*cp2 & 0xff) & 0xe0) == 0xc0) { 
				cp2+=2;
			} else if (((*cp2 & 0xff) & 0xf0) == 0xe0) { 
				cp2+=3;
			} else if (((*cp2 & 0xff) & 0xf8) == 0xf0) { 
				cp2+=4;
			} else if (((*cp2 & 0xff) & 0xfc) == 0xf8) { 
				cp2+=5;
			} else if (((*cp2 & 0xff) & 0xfe) == 0xfc) { 
				cp2+=6;
			}
		} else {
			space_found = 0;
		}
	} 
	*cp1 = '\0';

	return(ret);
}

/****************************************************************
 * fm_cust_pol_getutf8canon()
 * Maps input UTF8 character to Canonical euivalent UTF8 char.
 * 
 * unsigned char *cp1	output UTF8 Canon equivalent string 
 * unsigned char *cp2	input UTF8 string
 * 
 * return value: number of bytes in canonical char
 *
 ****************************************************************/
static int 
fm_cust_pol_getutf8canon(
	unsigned char 	*cp1, 
	unsigned char 	*cp2)
{
	int		i;
	int		j; 
	int		char_len = 0;
	int32		char_value = 0; 
	int32		canon_value = 0;
	int32		cp = 0;

	if ( !cp2  || !cp1 ) {
		return 0;	
	} else if (((*cp2 & 0xff) & 0x80) == 0x00) { 
		char_len=1;
	} else if (((*cp2 & 0xff) & 0xe0) == 0xc0) { 
		char_len=2;
	} else if (((*cp2 & 0xff) & 0xf0) == 0xe0) { 
		char_len=3;
	} else if (((*cp2 & 0xff) & 0xf8) == 0xf0) { 
		char_len=4;
	} else if (((*cp2 & 0xff) & 0xfc) == 0xf8) { 
		char_len=5;
	} else if (((*cp2 & 0xff) & 0xfe) == 0xfc) { 
		char_len=6;
	} else {
		return 0;	
	}

	/***********************************************************
	 * Gather all following bytes of UTF8 character and 
	 * convert into decimal value 
	 ***********************************************************/
	for (i=char_len-1, j=0; i>=0; i--, j++) {
		if (j==0) {
			char_value+=(int)(*(cp2+i));
		} else {
			char_value+=(int)(*(cp2+i)) * pow(256,j);
		}
	}

	/***********************************************************
	 * Map decimal value to the canon table
	 * Values < 128 map directly
	 ***********************************************************/
	
	if (char_value<128){
		canon_value=trans_UTF8_canon[256+char_value];
	} else {
		for (i=128; i<256; i++) {
			if (trans_UTF8_canon[i]==char_value){
				canon_value=trans_UTF8_canon[256+i];
				break;
			}
		}
	} 

	/***********************************************************
	 * If Canon value same as char value, just copy characters
	 * If Canon value NOT same as char, Convert canon to char
	 ***********************************************************/
	if (canon_value==char_value) {
		strncpy((char*)cp1, (char*)cp2, char_len);
		return (char_len);
	} else { 	
		char_len=1;
		while(1) {
			*cp1=canon_value%256;
			cp = *cp1;
			if (cp < 256) {
				break;
			}
			else {
				cp1++;
				char_len++;
			}
		} 
	}

	return(char_len);
}

/****************************************************************
 * Convert the input locale to canonicalized locale string.
 ****************************************************************/
static char * 
fm_cust_pol_convert_locale (char *inlocalep)
{
	struct locale_to_lcid	*cnp = NULL;
	char 			*localep = NULL;
	static char     	rbuf[32];

	static struct locale_to_lcid *id_to_lc_end = 
		 &(locale_to_lcid[ sizeof(locale_to_lcid) /
			 sizeof(struct locale_to_lcid) ] );

	/***********************************************************
	 * First check against the list of known locales and
	 * return the corresponding canonicalized locale.
	 ***********************************************************/
	for (cnp = locale_to_lcid; cnp < id_to_lc_end; cnp++ ) {
		if (!strcasecmp(cnp->suplocalesp, inlocalep)) { 
			localep = cnp->portallocalep;
			break;
		} 
	}

	if (localep != (char *)NULL) {
		return (localep);
	} else {
		return NULL;
	}
}
