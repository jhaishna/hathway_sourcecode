###########
#
# @(#)% % 
#
# Copyright (c) 1999, 2017, Oracle and/or its affiliates. All rights reserved.
#
#      This material is the confidential property of Oracle Corporation or its
#      licensors and may be used, reproduced, stored or transmitted only in
#      accordance with a valid Oracle license or sublicense agreement.
#
###########
OS=linux
VERSION=7.5

###########
PINDIR=../../../..
INCDIR=$(PINDIR)/include
LIBDIR=$(PINDIR)/lib
MSO_DIR=$(PINDIR)/mso
MSO_INCDIR=$(MSO_DIR)/include

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
CFLAGS= $(CFLAGS_$(OS)) -DFOR_CM -DFM_INV_POL_DLL

CPPFLAGS = -I$(INCDIR) -I$(MSO_INCDIR) -DPCMCPP_CONST_SAFE -DFOR_CM  -DFM_INV_POL_DLL

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
	$(INCDIR)/pin_flds.h $(INCDIR)/pin_currency.h \
	$(INCDIR)/pin_inv.h

#

FILES= fm_inv_pol_config.c \
	fm_inv_pol_init.c\
	fm_inv_pol_common.c\
	fm_inv_pol_format_view.c\
	fm_inv_pol_prep_invoice.c \
	fm_inv_pol_format_invoice.c \
	fm_inv_pol_format_invoice_doc1.c \
	fm_inv_pol_format_invoice_html.c \
	fm_inv_pol_format_invoice_xml.c \
	fm_inv_pol_format_invoice_xslt.c \
	fm_inv_pol_select.c \
	fm_inv_pol_post_make_invoice.c


#

OBJECTS= fm_inv_pol_config.o \
	fm_inv_pol_init.o\
	fm_inv_pol_common.o\
	fm_inv_pol_format_view.o\
	fm_inv_pol_prep_invoice.o \
	fm_inv_pol_format_invoice.o \
	fm_inv_pol_format_invoice_doc1.o \
	fm_inv_pol_format_invoice_html.o \
	fm_inv_pol_format_invoice_xml.o \
	fm_inv_pol_format_invoice_xslt.o \
	fm_inv_pol_select.o \
	fm_inv_pol_post_make_invoice.o

LIBBILL=fm_inv_pol_custom.$(SL_EXT)

###########

all: $(LIBBILL)

clean:
	rm -f $(OBJECTS) core

clobber: clean
	rm -f $(LIBBILL)

lint:
	lint $(CPPFLAGS) $(FILES)

tags: FRC

###########

$(LIBBILL): $(OBJECTS) $(INCFILES) Makefile
	$(LD) -o $(LIBBILL) $(LDFLAGS) $(OBJECTS) -lm -lpsiu_for_cm

$(OBJECTS): $(INCFILES) Makefile
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(FILES)

#

FRC:

