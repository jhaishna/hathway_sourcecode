/*******************************************************************
 *
* Copyright (c) 1999, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/ 

#ifndef lint
static  char Sccs_Id[] = "@#$Id: fm_cust_pol_prep_passwd.c /cgbubrm_mainbrm.portalbase/1 2019/02/25 22:33:19 agangrad Exp $"; 
#endif

#include <stdio.h>
#include <stdlib.h> 
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef MSDOS
#include <sys/socket.h>
#include <sys/timeb.h>
#endif
 
#ifdef __SVR4
#include <sys/systeminfo.h>	/* no gethostid() in Solaris */
#endif /*__SVR4*/

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"

#define FM_PWD_USE_PASSWD       8
#define FM_PWD_OK_CHARS "@#$%?23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ"
#define FM_PWD_DIGIT_CHARS   "23456789"
#define FM_PWD_SPECIAL_CHARS "@#$%?"
#define FM_PWD_UPPER_CHARS   "ABCDEFGHJKLMNPQRSTUVWXYZ"
#define FM_PWD_LOWER_CHARS   "abcdefghijkmnopqrstuvwxyz"

#ifdef MSDOS
#define lrand48 rand
#define srand48 srand

/*
void gettimeofday(struct timeval	*timeval, struct timezone *tz)
{
   struct _timeb timebuffer;

   _ftime( &timebuffer );
   timeval->tv_sec = timebuffer.time;
   timeval->tv_usec = 0;
}
*/

int getuid()
{
	return 2;
}

int gethostid()
{
	return 3;
}

#endif

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_prep_passwd(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_prep_passwd(
	pcm_context_t		*ctxp,
        int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_prep_pwd_acct(
	pcm_context_t		*ctxp,
	pin_flist_t		*r_flistp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_prep_pwd_srvc(
	pcm_context_t		*ctxp,
	int32                   flags,
	pin_flist_t		*r_flistp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_prep_pwd_gen(
	char		*passwd,
	int		pwd_length,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_prep_pwd_gsm(
	pcm_context_t           *ctxp,
	int32                   flags,
	pin_flist_t             *r_flistp,
	pin_errbuf_t            *ebufp);

static void
fm_cust_pol_prep_pwd_gsm_gen(
	char            *passwd,
	int32           pwd_length,
	pin_errbuf_t    *ebufp);


static void
fm_cust_pol_prep_pwd_tcf(
	pcm_context_t		*ctxp,
        int32		        flags,
	pin_flist_t		*r_flistp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_prep_pwd_tcf_gen(
	char		*passwd,
	int32		pwd_length,
	pin_errbuf_t	*ebufp);
/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_PREP_PASSWD  command
 *******************************************************************/
void
op_cust_pol_prep_passwd(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
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
	if (opcode != PCM_OP_CUST_POL_PREP_PASSWD) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_prep_passwd", ebufp);
		return;
	}

	/***********************************************************
	 * We will not open any transactions with Policy FM
	 * since policies should NEVER modify the database.
	 ***********************************************************/

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_prep_passwd(ctxp, flags, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_prep_passwd error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_prep_passwd return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_prep_passwd()
 *
 *	Prep the passwd to be ready for on-line registration.
 *	Our action depends on the type of object using the passwd.
 *	Specifically, we care about service passwds but force the
 *	account level passwds to NULL.
 *
 *******************************************************************/
static void
fm_cust_pol_prep_passwd(
	pcm_context_t		*ctxp,
        int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*r_flistp = NULL;

	poid_t			*o_pdp;
	const char		*o_type = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	r_flistp = PIN_FLIST_COPY(in_flistp, ebufp);
	*out_flistpp = r_flistp;

	/***********************************************************
	 * Get the object poid.
	 ***********************************************************/
	o_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);

	/***********************************************************
	 * What type of object?
	 ***********************************************************/
	o_type = PIN_POID_GET_TYPE(o_pdp);

	if (o_type && !strncmp(o_type, "/account", 8)) {

		/* Account level password */
		fm_cust_pol_prep_pwd_acct(ctxp, r_flistp, ebufp);

	} else if (o_type && !strncmp(o_type, "/service", 8)) {
		/***************************************************
		 * if telco service, we auto generate the 
		 * password, the firstime
		 ***************************************************/
		if (!strncmp(o_type, "/service/gsm", 12)) {
			fm_cust_pol_prep_pwd_gsm(ctxp, flags, r_flistp, ebufp);
		} 
		else if (!strncmp(o_type, "/service/telco",14)) {
			fm_cust_pol_prep_pwd_tcf(ctxp, flags, r_flistp, ebufp);
		}
		else if (flags & PCM_OPFLG_CUST_CREATE_ROLE){
                        fm_cust_pol_prep_pwd_srvc(ctxp, flags, r_flistp, ebufp);
                } else {
                        /* Service level password */
                        if ((strncmp(o_type, "/service/admin_client", 21)) &&
                        (strncmp(o_type, "/service/pcm_client", 19))) {
                                fm_cust_pol_prep_pwd_srvc(ctxp, flags,
                                        r_flistp, ebufp);
                        }
                }

	} else {

		/* Error - unknown/usupported type */
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_POID_TYPE, PIN_FLD_POID, 0, 0);

	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_passwd error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_prep_pwd_acct():
 *
 *	Prep the account level passwd for registration.
 *
 *	Forces all account level passords to be NULL.
 *
 *******************************************************************/
static void
fm_cust_pol_prep_pwd_acct(
	pcm_context_t		*ctxp,
	pin_flist_t		*r_flistp,
        pin_errbuf_t		*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Force account passwd to NULL.
	 ***********************************************************/
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_PASSWD_CLEAR, (void *)NULL, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_pwd_acct error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_prep_pwd_srvc():
 *
 *	Prep the service level passwd for registration.
 *
 *	Basically, we just see if one is passed in or not. If
 *	so, we do nothing, but if not, we generate one. Note that
 *	we consider the passwd passed in if the PASSWD_CLEAR field
 *	is on the input flist, but don't care about the value it
 *	might contain (ie it could be NULL). The value itself is
 *	checked by the _VALID_PASSWD call.
 *
 *******************************************************************/
static void
fm_cust_pol_prep_pwd_srvc(
	pcm_context_t		*ctxp,
	int32                   flags,
	pin_flist_t		*r_flistp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_flistp = NULL; 
	pin_flist_t		*tmp_flistp = NULL; 
	char			pwd_gen[FM_PWD_USE_PASSWD + 1];
	char			*loginid = NULL;
	char			tmppwd[255] = "";
	char			tmpstr[255] = "";
	char			tmploginid[255] = "";
	int32			PASSWD_PART = 3;
	int32			flag = PIN_BOOLEAN_FALSE;
	int32			i = 0;
	void                    *vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * See if a password was passed in.
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PASSWD_CLEAR, 0, ebufp);

	/***************************************************
        * Generate the passwd
        * Add the clear text passwd to the input list.
        ***************************************************/
        if (!vp || (flags & PCM_OPFLG_CUST_CREATE_ROLE)) {

		read_flistp = PIN_FLIST_CREATE(ebufp);
		vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_POID, 0, ebufp);	
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, vp, ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_LOGIN, NULL, ebufp);
	
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 
			PCM_OPFLG_CACHEABLE, read_flistp, &tmp_flistp, ebufp);	
		PIN_FLIST_DESTROY_EX(&read_flistp, NULL);

		loginid = PIN_FLIST_FLD_GET(tmp_flistp, 
			PIN_FLD_LOGIN, 0, ebufp);	

		while (flag == PIN_BOOLEAN_FALSE) { 
			flag = PIN_BOOLEAN_TRUE;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"Auto-generating password..");
			fm_cust_pol_prep_pwd_gen(pwd_gen,
				FM_PWD_USE_PASSWD, ebufp);
			
			if(loginid){
				for (i = 0; i < strlen(loginid); i++) {
					tmploginid[i] = tolower(loginid[i]);
				}
			}
			tmploginid[i] = '\0';

			for (i = 0; i < strlen(pwd_gen); i++) {
				tmppwd[i] = tolower(pwd_gen[i]);
			}
			tmppwd[i] = '\0';
			
			for (i = 0; i <= strlen(tmppwd) - PASSWD_PART; i++) {
				strncpy(tmpstr, &tmppwd[i], PASSWD_PART); 
				tmpstr[PASSWD_PART] = '\0';

				if (strstr(tmploginid, tmpstr) != NULL) {
					flag = PIN_BOOLEAN_FALSE;
					break;	
				}
			}
		}

                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_PASSWD_CLEAR,
                        (void *)pwd_gen, ebufp);

		PIN_FLIST_DESTROY_EX(&tmp_flistp, NULL);
        }

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_pwd_srvc error", ebufp);
	}

	return;
}


/*******************************************************************
 * fm_cust_pol_prep_pwd_gen():
 *
 *	Generates a random string of the requested length to
 *	be used as a unix style passwd.
 *
 *******************************************************************/
static void
fm_cust_pol_prep_pwd_gen(
	char		*passwd,
	int		pwd_length,
	pin_errbuf_t	*ebufp)
{
	int		c = 0;
	int		c1 = 0;
	int		c2 = 0;
	int		c3 = 0;
	int		cr = 0;
	int		i = 0;
	int		special_char = 5; 
	int		numbers = 8;
	int		small = 25;
	int		caps = 24;
	static  int	first_time = 1;
        struct timeval	timeval;
        /*long		lrand48();*/

        char		*ok_chars = FM_PWD_OK_CHARS;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
         * The first time through we use lots of different values
	 * to get the random number generators into an unknown state.
	 * We blend the outputs of two different generators to
	 * further mess things up.
	 ***********************************************************/
        if (first_time) {
#ifdef __SVR4
		char	buf[256];
		long	id;
#endif /*__SVR4*/
                gettimeofday(&timeval,(struct timezone *) 0);
 
                srand48((int) timeval.tv_sec);
                srand48(lrand48());
 
                gettimeofday(&timeval,(struct timezone *) 0);
                srand48((int) (lrand48() ^ timeval.tv_usec));
                gettimeofday(&timeval,(struct timezone *) 0);
                srand48(lrand48() ^ timeval.tv_usec);
 
                srand48((int) lrand48() ^ getpid());
                srand48(lrand48() ^ getpid());
 
#ifdef __SVR4
		id = sysinfo(SI_HW_SERIAL, buf, 256);
		if (id > 0) {
			sscanf(buf, "%ld", &id);
		} else {
			id = time((time_t *)0);
		}
                srand48((int) (lrand48() ^ id));
                srand48(lrand48() ^ id);
#else /*! __SVR4*/
                srand48((int) (lrand48() ^ gethostid()));
                srand48(lrand48() ^ gethostid());
#endif /*! __SVR4*/
 
                srand48((int) lrand48() ^ getuid());
                srand48(lrand48() ^ getuid());

                first_time = 0;
        }

	/***********************************************************
         * Each time we get called, we blend the current time to
	 * further permute the generators. This helps to keep us
	 * from getting predictable passwords, even if we know the
	 * starting conditions.
	 ***********************************************************/
        gettimeofday(&timeval,(struct timezone *) 0);
        srand48((int) (lrand48() ^ timeval.tv_sec));
        gettimeofday(&timeval,(struct timezone *) 0);
        srand48(lrand48() ^ timeval.tv_sec);

        gettimeofday(&timeval,(struct timezone *) 0);
        srand48(lrand48() ^ timeval.tv_usec);
        gettimeofday(&timeval,(struct timezone *) 0);
        srand48((int) (lrand48() ^ timeval.tv_usec));
 
	/***********************************************************
         *  Generate a unique string.
	 *
	 * If we get a prohibited character, try again. Password
	 * strings might be passed into ed (or similar) as part of
	 * printing say welcome letters, so characters that are
	 * special within ed patterns cannot be used.
	 ***********************************************************/
        i = 0;
        while (i < pwd_length) {
                if      (i == 0) ok_chars = FM_PWD_UPPER_CHARS;
                else if (i == 1) ok_chars = FM_PWD_LOWER_CHARS;
                else if (i == 2) ok_chars = FM_PWD_SPECIAL_CHARS;
                else if (i == 3) ok_chars = FM_PWD_DIGIT_CHARS;
                else             ok_chars = FM_PWD_OK_CHARS;

		srand(time(NULL));
		c = ((lrand48() ^ lrand48()) % strlen(ok_chars));
		passwd[i] = ok_chars[c];
		i++;
        }

	passwd[pwd_length] = '\0';
	/***********************************************************
	* Make sure that generated password has atleast one
	* special character, one lowecase and one uppercase
	* character and one digit.
	***********************************************************/

	/* Insert special character at random position */
	c = ((lrand48() ^ lrand48()) % strlen(passwd));
	cr = ((lrand48() ^ lrand48()) % special_char);
	passwd[c] = ok_chars[cr];

	/* Insert number at random position */
	do {
		srand(time(NULL));
		c1 = ((lrand48() ^ lrand48()) % strlen(passwd));
	} while (c1 == c);
	cr = ((lrand48() ^ lrand48()) % numbers);
	passwd[c1] = ok_chars[special_char + cr];

	/* Insert lowercase character at random position */
	do {
		srand(time(NULL));
		c2 = ((lrand48() ^ lrand48()) % strlen(passwd));
	} while ((c2 == c) || (c2 == c1));
	cr = ((lrand48() ^ lrand48()) % small);
	passwd[c2] = ok_chars[special_char + numbers+cr];

	/* Insert uppercase character at random position */
	do {
		srand(time(NULL));
		c3 = ((lrand48() ^ lrand48()) % strlen(passwd));
	} while ((c3 == c) || (c3 == c1) || (c3 == c2));
	cr = ((lrand48() ^ lrand48()) % caps);
	passwd[c3] = ok_chars[special_char + numbers + small + cr];


	/***********************************************************
	 * No errors.
	 ***********************************************************/
	PIN_ERR_CLEAR_ERR(ebufp);

	return;
}

/*******************************************************************
 * fm_cust_pol_prep_pwd_gsm():
 *
 *      Specific /service/gsm password during registration.
 *      We systematically set the password to the value returned by
 *      fm_cust_pol_prep_pwd_gsm_gen during customer registration.
 *      This password can be changed later thru webkit for example.
 *
 *******************************************************************/
static void
fm_cust_pol_prep_pwd_gsm(
	pcm_context_t           *ctxp,
	int32                   flags,
	pin_flist_t             *r_flistp,
	pin_errbuf_t            *ebufp)
{
	char                    pwd_gen[FM_PWD_USE_PASSWD + 1];
	char                    *current_passwd;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/* if it's the creation of the service */
	if (flags & PCM_OPFLG_CUST_REGISTRATION){
		/***************************************************
		 * Generate the passwd
		 ***************************************************/
		fm_cust_pol_prep_pwd_gsm_gen(pwd_gen,
					FM_PWD_USE_PASSWD,
					ebufp);
		/***************************************************
		 * Add the clear text passwd to the input list.
		 ***************************************************/
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_PASSWD_CLEAR,
			(void *)pwd_gen, ebufp);
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_pwd_gsm error", ebufp);
	}

	return;
}


/*******************************************************************
 * fm_cust_pol_prep_pwd_gsm_gen():
 *
 *      Generates a unique default password for all /service/gsm
 *
 *******************************************************************/
static void
fm_cust_pol_prep_pwd_gsm_gen(
	char            *passwd,
	int32           pwd_length,
	pin_errbuf_t    *ebufp)
{

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 *  Return a unique default password for all /service/gsm
	 ***********************************************************/
	strcpy(passwd, "password");

	return;
}



/*******************************************************************
 * fm_cust_pol_prep_pwd_tcf():
 *
 *	Specific /service/telco password during registration.
 *      We systematically set the password to the value returned by
 *      fm_cust_pol_prep_pwd_tcf_gen during customer registration.
 *      This password can be changed later thru webkit for example.
 *
 *******************************************************************/
static void
fm_cust_pol_prep_pwd_tcf(
	pcm_context_t		*ctxp,
        int32			flags,
	pin_flist_t		*r_flistp,
        pin_errbuf_t		*ebufp)
{
	char			pwd_gen[FM_PWD_USE_PASSWD + 1];
	char                    *current_passwd;
	
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/* if it's the creation of the service */
	if (flags & PCM_OPFLG_CUST_REGISTRATION){
		/***************************************************
		 * Generate the passwd
		 ***************************************************/
		fm_cust_pol_prep_pwd_tcf_gen(pwd_gen,
					     FM_PWD_USE_PASSWD, 
					     ebufp);
		/***************************************************
		 * Add the clear text passwd to the input list.
		 ***************************************************/
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_PASSWD_CLEAR,
				  (void *)pwd_gen, ebufp);
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_pwd_tcf error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_prep_pwd_tcf_gen():
 *
 *	Generates a unique default password for all /service/telco
 *
 *******************************************************************/
static void
fm_cust_pol_prep_pwd_tcf_gen(
	char		*passwd,
	int32		pwd_length,
	pin_errbuf_t	*ebufp)
{

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
     *  Return a unique default password for all /service/telco
	 ***********************************************************/
	strcpy(passwd, "password");

	return;
}
