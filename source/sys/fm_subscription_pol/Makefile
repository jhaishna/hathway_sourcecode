#
# @(#)% %
#
# Copyright (c) 2003, 2017, Oracle and/or its affiliates. All rights reserved.
#
#      This material is the confidential property of Oracle Corporation
#      or its subsidiaries or licensors and may be used, reproduced, stored
#      or transmitted only in accordance with a valid Oracle license or
#      sublicense agreement.
#

OS=linux
VERSION=7.5


####

PINDIR=../../../..
INCDIR=$(PINDIR)/include
MSO_DIR=$(PINDIR)/mso
MSO_INCDIR=$(MSO_DIR)/include

###########


##########
CC_solaris = cc
CC_hpux = cc
CC_hpux_ia64 = cc
CC_aix = xlc_r
CC_linux = gcc
CC = $(CC_$(OS))

C++_solaris = CC
C++_hpux = aCC
C++_hpux_ia64 = aCC
# not sure about aix
C++_aix = xlc_r 
C++_linux = gcc
C++ = $(C++_$(OS))
##########

CFLAGS_solaris= -g -xcg92
CFLAGS_hpux= -g -Ae +Z +DAportable
CFLAGS_hpux_ia64= -g -Ae +Z 
CFLAGS_aix= -g -Drs6000 -D__aix -Dunix -D__unix
CFLAGS_linux = -m32 -fPIC 

CFLAGS= $(CFLAGS_$(OS))

C++FLAGS_solaris= -g -library=%none -DPIN_NOT_USING_OSTREAM
C++FLAGS_hpux= -g -AA +Z +DAportable +nostl -DPIN_NOT_USING_OSTREAM -I/usr/include -I/opt/aCC/include -I/usr 
C++FLAGS_hpux_ia64= -g -AA +Z +nostl -DPIN_NOT_USING_OSTREAM -I/usr/include -I/opt/aCC/include -I/usr
C++FLAGS_aix= -g -Drs6000 -D__aix -Dunix -D__unix
C++FLAGS_linux = -x c++ -m32 -DPIN_NOT_USING_OSTREAM

C++FLAGS= $(C++FLAGS_$(OS)) -DPCMCPP_CONST_SAFE

CPPFLAGS = -I$(INCDIR) -I$(MSO_INCDIR) -DFOR_CM -DFM_SUBSCRIPTION_POL_DLL

LDFLAGS_solaris= -G -library=%none 
LDFLAGS_hpux= -b
LDFLAGS_hpux_ia64= -b
LDFLAGS_aix= -G -bnoentry -bexpall -brtl -lc
LDFLAGS_linux= -m32 -shared
LDFLAGS = $(LDFLAGS_$(OS))

SL_EXT_solaris= so
SL_EXT_hpux= sl
SL_EXT_hpux_ia64= so
SL_EXT_aix= a
SL_EXT_linux= so
SL_EXT= $(SL_EXT_$(OS))



INCFILES= $(INCDIR)/pin_cust.h $(INCDIR)/pcm.h \
	$(INCDIR)/pin_errs.h $(INCDIR)/pinlog.h \
	$(INCDIR)/pin_act.h $(INCDIR)/pin_bill.h \
	$(INCDIR)/pin_flds.h $(INCDIR)/pin_currency.h \
	$(INCDIR)/pin_subscription.h

#
FILES= fm_subscription_pol_config.c \
	fm_subscription_pol_config_eet.c \
	fm_subscription_pol_count_lines.c \
	fm_subscription_pol_get_sponsors.c \
	fm_subscription_pol_notify_aggregation.c \
	fm_subscription_pol_pre_fold.c \
	fm_subscription_pol_pre_transition_deal.c \
	fm_subscription_pol_pre_transition_plan.c \
	fm_subscription_pol_prep_fold.c \
	fm_subscription_pol_prep_members.c \
	fm_subscription_pol_provisioning.c \
	fm_subscription_pol_provisioning_content.c \
	fm_subscription_pol_provisioning_ssg.c \
	fm_subscription_pol_spec_cancel.c \
	fm_subscription_pol_spec_cancel_discount.c \
	fm_subscription_pol_spec_cycle_fee_interval.c \
	fm_subscription_pol_spec_fold.c \
	fm_subscription_pol_spec_rerate.c \
	fm_subscription_pol_update_cdc.c 
#	fm_subscription_pol_post_transfer_subscription_custom.c  

#

OBJECTS= fm_subscription_pol_config.o \
	fm_subscription_pol_config_eet.o \
	fm_subscription_pol_count_lines.o \
	fm_subscription_pol_get_sponsors.o \
	fm_subscription_pol_notify_aggregation.o \
	fm_subscription_pol_pre_fold.o \
	fm_subscription_pol_pre_transition_deal.o \
	fm_subscription_pol_pre_transition_plan.o \
	fm_subscription_pol_prep_fold.o \
	fm_subscription_pol_prep_members.o \
	fm_subscription_pol_provisioning.o \
	fm_subscription_pol_provisioning_content.o \
	fm_subscription_pol_provisioning_ssg.o \
	fm_subscription_pol_spec_cancel.o \
	fm_subscription_pol_spec_cancel_discount.o \
	fm_subscription_pol_spec_cycle_fee_interval.o \
	fm_subscription_pol_spec_fold.o \
	fm_subscription_pol_spec_rerate.o \
	fm_subscription_pol_update_cdc.o 
#	fm_subscription_pol_post_transfer_subscription_custom.o 

C++_FILES= fm_subscription_pol_init.cpp \
	fm_subscription_pol_snowball_discount.cpp

C++_OBJECTS= fm_subscription_pol_init.o \
	fm_subscription_pol_snowball_discount.o

LIBSUBS=fm_subscription_pol_custom.$(SL_EXT)

###########

all: $(LIBSUBS)

clean:
	rm -f $(OBJECTS) core

clobber: clean
	rm -f $(LIBSUBS)

lint:
	lint $(CPPFLAGS) $(FILES)

tags: FRC

###########

$(LIBSUBS): $(OBJECTS) $(C++_OBJECTS) $(INCFILES) Makefile
	$(C++) -o $(LIBSUBS) $(LDFLAGS) $(OBJECTS) $(C++_OBJECTS) -lm

$(OBJECTS): $(INCFILES) Makefile
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(FILES)

$(C++_OBJECTS): $(INCFILES) Makefile $(C++_FILES)
	$(C++) -c $(C++FLAGS) $(CPPFLAGS) $(C++_FILES)

#fm_subscription_pol_init.o: $(INCFILES) Makefile
#	$(C++) -c $(C++FLAGS) $(CPPFLAGS) fm_subscription_pol_init.cpp

#fm_subscription_pol_snowball_discount.o: $(INCFILES) Makefile
#	$(C++) -c $(C++FLAGS) $(CPPFLAGS) fm_subscription_pol_snowball_discount.cpp

#

FRC:

