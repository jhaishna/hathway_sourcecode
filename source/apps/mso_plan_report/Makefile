#
#	@(#) % %
#
# Copyright (c) 1996, 2012, Oracle and/or its affiliates. All rights reserved. 
#      
#       This material is the confidential property of Oracle Corporation or its
#       licensors and may be used, reproduced, stored or transmitted only in
#       accordance with a valid Oracle license or sublicense agreement.
#
#

OS=linux
PIN_VERSION=7.5

##########

PINDIR=$(PIN_HOME)
INCDIR=$(PINDIR)/include
LIBDIR=$(PINDIR)/lib

##########
CC_comp_solaris = cc
CC_link_solaris = CC
CC_comp_linux = gcc
CC_link_linux = gcc
CC_comp_hpux = cc
CC_link_hpux = aCC
CC_comp_aix = xlc_r
CC_link_aix = $(CC_comp_aix)
CC_comp = $(CC_comp_$(OS))
CC_link = $(CC_link_$(OS))

##########
CFLAGS_solaris= -g -xcg92
CFLAGS_linux= -m32 -fPIC
CFLAGS_hpux= -g -Ae +Z
CFLAGS_aix= -D rs6000 -D __aix -D unix -D DEBUG_AIX  -g -D PIN_USE_ANSI_HDRS -D__unix -qchars=signed
CFLAGS= $(CFLAGS_$(OS))

CPPFLAGS = -I$(INCDIR)

LDFLAGS_solaris= -R:$(LIBDIR): 
LDFLAGS_linux= -m32
LDFLAGS_hpux= -Wl,-E -Wl,+b:$(LIBDIR):
LDFLAGS_aix= -G -bnoentry -lc
LDFLAGS = -L$(LIBDIR) $(LDFLAGS_$(OS))

OS_LIBS_solaris= -lsocket -lnsl -lgen
OS_LIBS_hpux= -lsec

LDLIBS= -lportal $(OS_LIBS_$(OS))

##########

INCFILES= $(INCDIR)/pcm.h \
	$(INCDIR)/pin_bill.h $(INCDIR)/pin_cust.h $(INCDIR)/pin_act.h \
	$(INCDIR)/pin_errs.h $(INCDIR)/pinlog.h

###########

APPS= mso_plan_report

OBJECTS= mso_plan_report.o 

###########

all: $(APPS)

clean:
	rm -f $(OBJECTS) core

clobber: clean
	rm -f $(APPS)

###########

$(APPS): $(OBJECTS) $(INCFILES) Makefile
	$(CC_link) -o $@ $(CPPFLAGS) $(LDFLAGS) $@.o $(LDLIBS)

$(OBJECTS): $(INCFILES) Makefile
	$(CC_comp) -c $(CFLAGS) $(CPPFLAGS) $*.c

###########

FRC:

