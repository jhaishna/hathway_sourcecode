###########
#
# @(#)% Make.user (4) %
#
# Copyright (c) 1996, 2013, Oracle and/or its affiliates. All rights reserved. 
#
#	This material is the confidential property of Oracle Corporation or its
#	licensors and may be used, reproduced, stored or transmitted only in
#	accordance with a valid Oracle license or sublicense agreement.
#
###########

OS=linux
VERSION=7.5

####

PINDIR=../../../..
INCDIR=$(PINDIR)/include
MSO_INCDIR=$(PINDIR)/mso/include
LIBDIR=$(PINDIR)/lib

##########
CC_solaris = cc
CC_hpux = cc
CC_hpux_ia64 = cc
CC_aix = xlc_r
CC_linux = gcc
CC = $(CC_$(OS))

##########

CFLAGS_solaris= -g -xcg92
CFLAGS_hpux= -g -Ae +Z +DAportable
CFLAGS_hpux_ia64= -g -Ae +Z 
CFLAGS_aix= -g -Drs6000 -D__aix -Dunix -D__unix
CFLAGS_linux= -m32 -fPIC
CFLAGS= $(CFLAGS_$(OS)) -DFOR_CM -DFM_PYMT_DLL

CPPFLAGS = -I$(INCDIR) -I$(MSO_INCDIR) -DPCMCPP_CONST_SAFE -DFOR_CM -DFM_PYMT_DLL

LDFLAGS_solaris= -G
LDFLAGS_hpux= -b
LDFLAGS_hpux_ia64= -b
LDFLAGS_aix= -G -bnoentry -bexpall -lc -brtl
LDFLAGS_linux= -m elf_i386 -shared -L/usr/lib
LDFLAGS = $(LDFLAGS_$(OS)) -L$(LIBDIR)

SL_EXT_solaris= so
SL_EXT_hpux= sl
SL_EXT_hpux_ia64= so
SL_EXT_aix= a
SL_EXT_linux= so
SL_EXT= $(SL_EXT_$(OS))

###########

INCFILES= $(INCDIR)/pin_cust.h $(INCDIR)/pcm.h \
	$(INCDIR)/pin_errs.h $(INCDIR)/pinlog.h \
	$(INCDIR)/pin_act.h $(INCDIR)/pin_bill.h \
	$(INCDIR)/pin_flds.h $(INCDIR)/pin_currency.h 

#

FILES=  fm_mso_pymt_config.c \
	fm_mso_pymt_collect.c \
	fm_mso_pymt_reverse_payment.c \
	fm_mso_pymt_deposit.c \
	fm_mso_pymt_deposit_refund.c \
	fm_mso_pymt_get_receipts.c \
	fm_mso_pymt_get_deposits.c \
	fm_mso_pymt_bulk_collect.c \
	fm_mso_pymt_lco_collect.c \
	fm_mso_pymt_blk_get_payments.c \
	fm_mso_pymt_lco_collect_update.c \
	fm_mso_pymt_allocate_payment.c \
	fm_mso_pymt_autopay.c \
	fm_mso_pymt_correction.c \
	fm_mso_pymt_process_refund.c \
	fm_mso_pymt_reverse_refund.c \
	fm_mso_ar_allocate.c


#

OBJECTS= fm_mso_pymt_config.o \
	fm_mso_pymt_collect.o \
	fm_mso_pymt_reverse_payment.o \
	fm_mso_pymt_deposit.o \
	fm_mso_pymt_deposit_refund.o \
	fm_mso_pymt_get_receipts.o \
	fm_mso_pymt_get_deposits.o \
	fm_mso_pymt_bulk_collect.o \
	fm_mso_pymt_lco_collect.o \
	fm_mso_pymt_blk_get_payments.o \
	fm_mso_pymt_lco_collect_update.o \
	fm_mso_pymt_allocate_payment.o \
	fm_mso_pymt_autopay.o \
	fm_mso_pymt_correction.o \
	fm_mso_pymt_process_refund.o \
	fm_mso_pymt_reverse_refund.o \
	fm_mso_ar_allocate.o

LIBPYMT=fm_mso_pymt_custom.$(SL_EXT)

###########

all: $(LIBPYMT)

clean:
	rm -f $(OBJECTS) core

clobber: clean
	rm -f $(LIBPYMT)

lint:
	lint $(CPPFLAGS) $(FILES)

tags: FRC


###########

$(LIBPYMT): $(OBJECTS) $(INCFILES) Makefile
	$(LD) -o $(LIBPYMT) $(LDFLAGS) $(OBJECTS) -lm -lpsiu_for_cm

$(OBJECTS): $(INCFILES) Makefile $(FILES)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(FILES)

#

FRC:

