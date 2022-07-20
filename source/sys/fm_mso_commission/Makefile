###########
#
# @(#)% %
#
# Copyright (c) 1999, 2010, Oracle and/or its affiliates. All rights reserved. 
#
#      This material is the confidential property of Oracle Corporation
#      or its licensors and may be used, reproduced, stored
#      or transmitted only in accordance with a valid Oracle license or
#      sublicense agreement.
#
###########

OS=linux
VERSION=7.5

##########

PINDIR=../../../..
INCDIR=$(PINDIR)/include
LIBDIR=$(PINDIR)/lib
MSO_DIR=$(PINDIR)/mso
MSO_INCDIR=$(MSO_DIR)/include

##########
CC_solaris = cc
CC_linux = gcc
CC_hpux = cc
CC_hpux_ia64 = cc
CC_aix = xlc_r
CC = $(CC_$(OS))

###########

CFLAGS_solaris= -g -xcg92
#CFLAGS_linux= -m32 -fPIC
CFLAGS_linux= -g -m32 -fPIC -Wimplicit-function-declaration -Wunused-variable -Wunused-function
CFLAGS_hpux= -g -Ae +Z +DAportable
CFLAGS_hpux_ia64= -g -Ae +Z 
CFLAGS_aix= -g -Drs6000 -D__aix -Dunix -D__unix
CFLAGS= $(CFLAGS_$(OS)) -DFLIST_HEAP

#CPPFLAGS = -I$(INCDIR)
CPPFLAGS = -I$(INCDIR) -I$(MSO_INCDIR)  -DPCMCPP_CONST_SAFE -DFOR_CM -DFM_PYMT_DLL

LDFLAGS_solaris= -G
LDFLAGS_linux= -m elf_i386 -shared -L/usr/lib
LDFLAGS_hpux= -b
LDFLAGS_hpux_ia64= -b
LDFLAGS_aix= -G -bnoentry -bexpall -lc
LDFLAGS = $(LDFLAGS_$(OS)) -L$(LIBDIR)

SL_EXT_solaris= so
SL_EXT_linux= so
SL_EXT_hpux= sl
SL_EXT_hpux_ia64= so
SL_EXT_aix= a
SL_EXT= $(SL_EXT_$(OS))

OS_LIBS_solaris= -lm
OS_LIBS_linux= -lm
OS_LIBS_hpux= -lsec -lm
OS_LIBS_aix= -lm
OS_LIBS= $(OS_LIBS_$(OS))

####

INCFILES= $(INCDIR)/pin_cust.h $(INCDIR)/pcm.h \
	$(INCDIR)/pin_errs.h $(INCDIR)/pinlog.h \
	$(INCDIR)/pin_flds.h $(INCDIR)/pin_currency.h \
	$(INCDIR)/pin_act.h $(INCDIR)/pin_bill.h

FILES= 	fm_mso_commission_config.c \
	fm_mso_commission_common_functions.c \
	fm_mso_commission_manage_agreement.c \
	fm_mso_commission_apply_commission.c \
	fm_mso_commission_process_commission.c \
	fm_mso_commission_lco_bal_transfer.c \
	fm_mso_commission_lco_bal_impact.c \
	fm_mso_commission_rectify_lco_wrong_tagging.c \
	fm_mso_commission_rectify_sdt_dt_wrong_tagging.c \
	fm_mso_commission_process_dsa_commission.c \
	fm_mso_commission_get_lco_plans.c


OBJECTS= fm_mso_commission_config.o \
	fm_mso_commission_common_functions.o \
	fm_mso_commission_manage_agreement.o \
	fm_mso_commission_apply_commission.o \
	fm_mso_commission_process_commission.o \
	fm_mso_commission_lco_bal_transfer.o \
	fm_mso_commission_lco_bal_impact.o \
	fm_mso_commission_rectify_lco_wrong_tagging.o \
	fm_mso_commission_rectify_sdt_dt_wrong_tagging.o \
	fm_mso_commission_process_dsa_commission.o \
	fm_mso_commission_get_lco_plans.o

LIBACT=fm_mso_commission.$(SL_EXT)

###########

#all: $(LIBACT)
all: $(LIBACT)

clean:
	rm -f $(OBJECTS) core

clobber: clean
	rm -f $(LIBACT)

lint:
	lint $(CPPFLAGS) $(FILES)

tags: FRC

###########

$(LIBACT): $(OBJECTS) $(INCFILES) Makefile
	$(LD) -o $(LIBACT) $(LDFLAGS) $(OBJECTS) $(OS_LIBS) -lpsiu_for_cm

$(OBJECTS): $(INCFILES) Makefile
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(FILES)

#


FRC:
