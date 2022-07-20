###########
#
# @(#)% full_filespec: Make.user~21 %
#
# Copyright (c) 1996, 2017, Oracle and/or its affiliates. All rights reserved.
#
#      This material is the confidential property of Oracle
#      Corporation or its licensors and may be used, reproduced,
#      stored or transmitted only in accordance with a valid
#      Oracle license or sublicense agreement.
#
###########

OS=linux
VERSION=7.5

##########

PINDIR=../../../..
INCDIR=$(PINDIR)/include
LIBDIR=$(PINDIR)/lib
MSO_INCDIR=$(PINDIR)/mso/include

##########
CC_solaris = cc
CC_hpux = cc
CC_hpux_ia64 = cc
CC_aix = xlc_r
CC_linux = gcc 
CC = $(CC_$(OS))

###########

CFLAGS_solaris= -g
CFLAGS_hpux= -g -Ae +Z +DAportable
CFLAGS_hpux_ia64= -g -Ae +Z
CFLAGS_aix= -g -Drs6000 -D__aix -Dunix -D__unix
CFLAGS_linux= -m32 -fPIC
CFLAGS= $(CFLAGS_$(OS)) -DFOR_CM -DFM_CUST_POL_DLL

CPPFLAGS = -I$(INCDIR) -I$(MSO_INCDIR) -DPCMCPP_CONST_SAFE -DFOR_CM -DFM_CUST_POL_DLL

LDFLAGS_solaris= -G
LDFLAGS_hpux= -b
LDFLAGS_hpux_ia64= -b
LDFLAGS_aix= -G -bnoentry -bexpall -brtl -lc
LDFLAGS_linux= -m elf_i386 -shared -L/usr/lib
LDFLAGS = $(LDFLAGS_$(OS)) -L$(LIBDIR)

SL_EXT_solaris= so
SL_EXT_hpux= sl
SL_EXT_hpux_ia64= so
SL_EXT_aix= a 
SL_EXT_linux= so
SL_EXT= $(SL_EXT_$(OS))

##########

INCFILES= $(INCDIR)/pin_cust.h $(INCDIR)/pcm.h \
	$(INCDIR)/pin_errs.h $(INCDIR)/pinlog.h

#
FILES= fm_cust_pol_config.c \
	fm_cust_pol_pre_commit.c \
	fm_cust_pol_init.c \
	fm_cust_pol_post_commit.c \
	fm_cust_pol_valid_all.c \
	fm_cust_pol_valid_acctinfo.c \
	fm_cust_pol_valid_billinfo.c \
	fm_cust_pol_valid_nameinfo.c \
	fm_cust_pol_valid_aacinfo.c \
	fm_cust_pol_valid_inherited.c \
	fm_cust_pol_valid_passwd.c \
	fm_cust_pol_valid_locale.c \
	fm_cust_pol_valid_login.c \
	fm_cust_pol_valid_limit.c \
	fm_cust_pol_valid_status.c \
	fm_cust_pol_prep_acctinfo.c \
	fm_cust_pol_prep_billinfo.c \
	fm_cust_pol_prep_nameinfo.c \
	fm_cust_pol_prep_aacinfo.c \
	fm_cust_pol_prep_inherited.c \
	fm_cust_pol_prep_passwd.c \
	fm_cust_pol_prep_locale.c \
	fm_cust_pol_prep_login.c \
	fm_cust_pol_prep_limit.c \
	fm_cust_pol_prep_status.c \
	fm_cust_pol_prep_topup.c \
	fm_cust_pol_get_poplist.c \
	fm_cust_pol_get_config.c \
	fm_cust_pol_get_intro_msg.c \
	fm_cust_pol_get_plans.c \
	fm_cust_pol_get_deals.c \
	fm_cust_pol_get_prods.c \
	fm_cust_pol_compare_passwd.c \
	fm_cust_pol_encrypt_passwd.c \
	fm_cust_pol_expiration_passwd.c \
	fm_cust_pol_encrypt_pwdmd5.c \
	fm_cust_pol_decrypt_passwd.c \
	fm_cust_pol_util.c \
	fm_cust_pol_prep_payinfo.c \
	fm_cust_pol_valid_payinfo.c \
	fm_cust_pol_prep_profile.c \
	fm_cust_pol_modify_service.c \
	fm_cust_pol_valid_profile.c \
	fm_cust_pol_valid_topup.c \
	fm_cust_pol_canonicalize.c \
	fm_cust_pol_get_db_list.c \
	fm_cust_pol_get_db_no.c \
	fm_cust_pol_valid_taxinfo.c \
	fm_cust_pol_read_plan.c \
	fm_cust_pol_tax_init.c \
	fm_cust_pol_tax_calc.c \
	fm_cust_pol_set_brandinfo.c \
	fm_cust_pol_post_modify_customer.c \
	fm_cust_pol_transition_deals.c \
	fm_cust_pol_transition_plans.c \
	fm_cust_pol_get_subscribed_plans.c
#       fm_cust_pol_pre_delete_payinfo_custom.c


OBJECTS= fm_cust_pol_config.o \
	fm_cust_pol_pre_commit.o \
	fm_cust_pol_init.o \
	fm_cust_pol_post_commit.o \
	fm_cust_pol_valid_all.o \
	fm_cust_pol_valid_acctinfo.o \
	fm_cust_pol_valid_billinfo.o \
	fm_cust_pol_valid_nameinfo.o \
	fm_cust_pol_valid_aacinfo.o \
	fm_cust_pol_valid_inherited.o \
	fm_cust_pol_valid_passwd.o \
	fm_cust_pol_valid_locale.o \
	fm_cust_pol_valid_login.o \
	fm_cust_pol_valid_limit.o \
	fm_cust_pol_valid_status.o \
	fm_cust_pol_prep_acctinfo.o \
	fm_cust_pol_prep_billinfo.o \
	fm_cust_pol_prep_nameinfo.o \
	fm_cust_pol_prep_aacinfo.o \
	fm_cust_pol_prep_inherited.o \
	fm_cust_pol_prep_passwd.o \
	fm_cust_pol_prep_locale.o \
	fm_cust_pol_prep_login.o \
	fm_cust_pol_prep_limit.o \
	fm_cust_pol_prep_status.o \
	fm_cust_pol_prep_topup.o \
	fm_cust_pol_get_poplist.o \
	fm_cust_pol_get_config.o \
	fm_cust_pol_get_intro_msg.o \
	fm_cust_pol_get_plans.o \
	fm_cust_pol_get_deals.o \
	fm_cust_pol_get_prods.o \
	fm_cust_pol_compare_passwd.o \
	fm_cust_pol_encrypt_passwd.o \
	fm_cust_pol_expiration_passwd.o \
	fm_cust_pol_encrypt_pwdmd5.o \
	fm_cust_pol_decrypt_passwd.o \
	fm_cust_pol_util.o \
	fm_cust_pol_prep_payinfo.o \
	fm_cust_pol_valid_payinfo.o \
	fm_cust_pol_prep_profile.o \
	fm_cust_pol_modify_service.o \
	fm_cust_pol_valid_profile.o \
	fm_cust_pol_valid_topup.o \
	fm_cust_pol_canonicalize.o \
	fm_cust_pol_get_db_list.o \
	fm_cust_pol_get_db_no.o \
	fm_cust_pol_valid_taxinfo.o \
	fm_cust_pol_read_plan.o \
	fm_cust_pol_tax_init.o \
	fm_cust_pol_tax_calc.o \
	fm_cust_pol_set_brandinfo.o \
	fm_cust_pol_post_modify_customer.o \
	fm_cust_pol_transition_deals.o \
	fm_cust_pol_transition_plans.o \
	fm_cust_pol_get_subscribed_plans.o
#	fm_cust_pol_pre_delete_payinfo_custom.o


#

LIBCUST=fm_cust_pol_custom.$(SL_EXT)

###########

all: $(LIBCUST)

clean:
	rm -f $(OBJECTS) core

clobber: clean
	rm -f $(LIBCUST)

lint:
	lint $(CPPFLAGS) $(FILES)

tags: FRC

###########

$(LIBCUST): $(OBJECTS) $(INCFILES) Makefile
	$(LD) -o $(LIBCUST) $(LDFLAGS) $(OBJECTS) -lpin_cc -lm -lpsiu_for_cm -lregex

$(OBJECTS): $(INCFILES) Makefile
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(FILES)

#

FRC:
