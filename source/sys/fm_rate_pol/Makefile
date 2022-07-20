###########
#
# @(#)% %
#
# Copyright (c) 1999, 2017, Oracle and/or its affiliates. All rights reserved.
#
#       This material is the confidential property of Oracle Corporation
#       or its subsidiaries or licensors and may be used, reproduced,
#       stored or transmitted only in accordance with a valid Oracle
#       license or sublicense agreement.
#
###########

OS=linux
VERSION=7.5

##########

PINDIR=../../../..
INCDIR=$(PINDIR)/include
MSO_INCDIR=$(PINDIR)/mso/include

##########
CC_solaris = cc
CC_linux = gcc
CC_hpux = cc
CC_hpux_ia64 = cc
CC_aix = xlc_r
CC = $(CC_$(OS))

##########
CFLAGS_solaris= -g -D_REENTRANT
CFLAGS_linux= -m32 -fPIC
CFLAGS_aix= -Drs6000 -Dunix -D__unix -D__aix
CFLAGS_hpux= -g -Ae +Z +DAportable
CFLAGS_hpux_ia64= -g -Ae +Z
CFLAGS= $(CFLAGS_$(OS)) -DFLIST_HEAP

CPPFLAGS = -I$(INCDIR) -I$(MSO_INCDIR)

LDFLAGS_solaris= -G
LDFLAGS_linux= -m elf_i386 -shared -L/usr/lib
LDFLAGS_aix= -G -bexpall -bnoentry -brtl -lc
LDFLAGS_hpux_ia64= -b
LDFLAGS_hpux= -b
LDFLAGS = $(LDFLAGS_$(OS))

SL_EXT_solaris= so
SL_EXT_linux= so
SL_EXT_aix= a
SL_EXT_hpux= sl
SL_EXT_hpux_ia64= so
SL_EXT= $(SL_EXT_$(OS))

###########

INCFILES= $(INCDIR)/pin_cust.h $(INCDIR)/pcm.h \
	$(INCDIR)/pin_errs.h $(INCDIR)/pinlog.h

#

FILES= fm_rate_pol_config.c \
	fm_rate_pol_tax_code.c \
	fm_rate_pol_tax_loc.c \
	fm_rate_pol_post_rating.c \
	fm_rate_pol_map_tax_supplier.c \
	fm_rate_pol_tax_supplier.c \
	fm_rate_pol_pre_tax.c \
	fm_rate_pol_post_tax.c  \
	fm_rate_pol_event_zonemap.c \
	fm_rate_pol_pre_rating.c \
	fm_rate_pol_init.c \
       fm_rate_pol_process_eras.c

OBJECTS= fm_rate_pol_config.o \
	fm_rate_pol_tax_code.o \
	fm_rate_pol_tax_loc.o \
	fm_rate_pol_post_rating.o \
	fm_rate_pol_map_tax_supplier.o \
	fm_rate_pol_tax_supplier.o \
	fm_rate_pol_pre_tax.o \
	fm_rate_pol_post_tax.o \
	fm_rate_pol_event_zonemap.o \
	fm_rate_pol_pre_rating.o \
	fm_rate_pol_init.o \
	fm_rate_pol_process_eras.o

#

LIBRATE=fm_rate_pol_custom.$(SL_EXT)

###########

all: $(LIBRATE)

clean:
	rm -f $(OBJECTS) core

clobber: clean
	rm -f $(LIBRATE)

lint:
	lint $(CPPFLAGS) $(FILES)

tags: FRC

###########

$(LIBRATE): $(OBJECTS) $(INCFILES) Makefile
	$(LD) -o $(LIBRATE) $(LDFLAGS) $(OBJECTS) -lm

$(OBJECTS): $(INCFILES) Makefile
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(FILES)

#

FRC:
