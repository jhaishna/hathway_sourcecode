#############################################################################
#
# @(#)$Id: Make.user /cgbubrm_mainbrm.portalbase/1 2017/09/06 00:13:43 ashutask Exp $
#
# Copyright (c) 2001, 2017, Oracle and/or its affiliates. All rights reserved.
#
#      This material is the confidential property of Oracle Corporation or its
#      licensors and may be used, reproduced, stored or transmitted only in
#      accordance with a valid Oracle license or sublicense agreement.
#
#############################################################################

OS=linux
VERSION=7.5

##########
CC_solaris = cc
CC_linux = gcc
CC_hpux = cc
CC_hpux_ia64 = cc
CC_aix = xlc_r
CC = $(CC_$(OS))

CFLAGS_solaris= -g -xcg92
CFLAGS_linux= -g -m32 -fPIC
CFLAGS_hpux= -g -Ae +Z +DAportable
CFLAGS_hpux_ia64= -g -Ae +Z
CFLAGS_aix= -g -Drs6000 -D__aix -Dunix -D__unix
CFLAGS= $(CFLAGS_$(OS)) -DFOR_CM -DFM_COLLECTIONS_POL_DLL

CPPFLAGS = -I$(INCDIR) -I$(MSO_INCDIR) -DPCMCPP_CONST_SAFE -DFOR_CM -DFM_COLLECTIONS_POL_DLL

LDFLAGS_solaris= -G
LDFLAGS_linux= -m elf_i386 -shared -L/usr/lib
LDFLAGS_hpux= -b
LDFLAGS_hpux_ia64= -b
LDFLAGS_aix= -G -bnoentry -bexpall -lc
LDFLAGS = $(LDFLAGS_$(OS))

SL_EXT_solaris= so
SL_EXT_linux= so
SL_EXT_hpux= sl
SL_EXT_hpux_ia64= so
SL_EXT_aix= a
SL_EXT= $(SL_EXT_$(OS))


####

PINDIR=../../../..
INCDIR=$(PINDIR)/include
MSO_INCDIR=$(PINDIR)/mso/include

###########

INCFILES= $(INCDIR)/pin_cust.h $(INCDIR)/pcm.h \
	$(INCDIR)/pin_errs.h $(INCDIR)/pinlog.h \
	$(INCDIR)/pin_act.h $(INCDIR)/pin_bill.h \
	$(INCDIR)/pin_flds.h $(INCDIR)/pin_currency.h \
	$(INCDIR)/fm_collections.h

#

FILES= fm_collections_pol_config.c \
	fm_collections_pol_exit_scenario.c\
	fm_collections_pol_exec_policy_action.c\
	fm_collections_pol_prep_dunning_data.c\
	fm_collections_pol_select_profile.c\
	fm_collections_pol_apply_late_fees.c \
	fm_collections_pol_assign_agent.c \
	fm_collections_pol_apply_finance_charges.c \
	fm_mso_collections_pol_apply_late_fees.c

#

OBJECTS= fm_collections_pol_config.o \
	fm_collections_pol_exit_scenario.o\
	fm_collections_pol_exec_policy_action.o\
	fm_collections_pol_prep_dunning_data.o\
	fm_collections_pol_select_profile.o\
	fm_collections_pol_apply_late_fees.o \
	fm_collections_pol_assign_agent.o \
	fm_collections_pol_apply_finance_charges.o \
	fm_mso_collections_pol_apply_late_fees.o

LIBCOLLECTIONS=fm_collections_pol_custom.$(SL_EXT)

###########

all: $(LIBCOLLECTIONS)

clean:
	rm -f $(OBJECTS) core

clobber: clean
	rm -f $(LIBCOLLECTIONS)

lint:
	lint $(CPPFLAGS) $(FILES)

tags: FRC

###########

$(LIBCOLLECTIONS): $(OBJECTS) $(INCFILES) Makefile
	$(LD) -o $(LIBCOLLECTIONS) $(LDFLAGS) $(OBJECTS) -lm

$(OBJECTS): $(INCFILES) Makefile
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(FILES)

#

FRC:

