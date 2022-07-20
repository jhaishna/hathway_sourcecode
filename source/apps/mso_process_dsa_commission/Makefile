###########
#
# @(#)Make.user 1.12 08/06/13
#
#      Copyright (c) 2003 - 2008 Oracle. All rights reserved.
#
#      This material is the confidential property of Oracle Corporation or its
#      licensors and may be used, reproduced, stored or transmitted only in
#      accordance with a valid Oracle license or sublicense agreement.
#
###########

OS=linux
VERSION=7.5

##########

PINDIR=$(PIN_HOME)
INCDIR=$(PINDIR)/include
MSOINCDIR=$(PINDIR)/mso/include
LIBDIR=$(PINDIR)/lib

##########
CC_comp_solaris = cc
CC_link_solaris = CC
CC_comp_linux = gcc
CC_link_linux= gcc
CC_comp_hpux_ia64 = aCC
CC_link_hpux_ia64 = $(CC_comp_hpux_ia64)
CC_comp_hpux = aCC
CC_link_hpux = $(CC_comp_hpux)
CC_comp_aix = xlc_r
CC_link_aix = $(CC_comp_aix)
CC_comp = $(CC_comp_$(OS))
CC_link = $(CC_link_$(OS))

##########
CFLAGS_solaris= -g -xcg92
CFLAGS_linux= -m32 -fPIC -g -pthread
CFLAGS_hpux_ia64= -g -Ae +Z
CFLAGS_hpux= -g -Ae +Z
CFLAGS_aix= -g -Drs6000 -Dunix -D__unix -D__aix
CFLAGS= $(CFLAGS_$(OS))

CPPFLAGS = -I$(INCDIR) -I$(MSOINCDIR)

LDFLAGS_solaris= -R:$(LIBDIR): 
LDFLAGS_linux= -m32 -Wl,--export-dynamic
LDFLAGS_hpux_ia64= -Wl,-E -Wl,+b:$(LIBDIR):
LDFLAGS_hpux= -Wl,-E -Wl,+b:$(LIBDIR):
LDFLAGS_aix=-bM:SRE -bexpall -brtl -lc
LDFLAGS = -L$(LIBDIR) $(LDFLAGS_$(OS))

OS_LIBS_solaris= -lsocket -lnsl -lgen -lposix4 -lpthread -ldl
OS_LIBS_hpux_ia64= -lsec -lrt -lpthread -lCsup -lunwind
OS_LIBS_hpux= -lsec -lrt -lpthread -lCsup
OS_LIBS_aix= -lpthread
OS_LIBS_linux= -lpthread -ldl

LDLIBS= -lportal -lmta -lpinsys $(OS_LIBS_$(OS))

##########

INCFILES= $(INCDIR)/pcm.h $(INCDIR)/pin_mta.h \
	$(INCDIR)/pin_bill.h $(INCDIR)/pin_cust.h $(INCDIR)/pin_act.h \
	$(INCDIR)/pin_errs.h $(INCDIR)/pinlog.h

###########

APPS= mso_process_dsa_commission

FILES= mso_process_dsa_commission.c

OBJECTS= mso_process_dsa_commission.o

DEP_linux=$(FILES)
DEP=$(DEP_$(OS))

###########

all: $(APPS)

clean:
	rm -f $(OBJECTS) core

clobber: clean
	rm -f $(APPS)

###########

$(APPS): $(OBJECTS) $(INCFILES) Makefile
	$(CC_link) -o $@ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $@.o $(LDLIBS)

$(OBJECTS): $(INCFILES) Makefile $(DEP)
	$(CC_comp) -c $(CFLAGS) $(CPPFLAGS) $*.c

###########

FRC:


