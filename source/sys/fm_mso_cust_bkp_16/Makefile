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

PINDIR=../../../../
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
#CFLAGS_linux= -g -m32 -fPIC
# removing -g flag
CFLAGS_linux= -g -m32 -fPIC 
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

FILES= 	fm_mso_cust_config.c \
	fm_mso_cust_oap_login.c \
	fm_mso_cust_register_customer.c \
	fm_mso_cust_activate_customer.c \
	fm_mso_cust_get_plan_details.c \
	fm_mso_cust_get_customer_info.c \
	fm_mso_cust_set_customer_bit.c \
	fm_mso_cust_modify_customer_credentials.c \
	fm_mso_cust_modify_customer.c \
	fm_mso_cust_suspend_service.c \
	fm_mso_cust_reactivate_service.c \
	fm_mso_cust_terminate_service.c \
	fm_mso_cust_change_plan.c \
	fm_mso_cust_cancel_plan.c \
	fm_mso_cust_add_plan.c \
	fm_mso_cust_get_invoice.c\
	fm_mso_cust_create_schedule.c \
	fm_mso_cust_get_device_details.c \
	fm_mso_cust_get_channel_and_price.c \
	fm_mso_search.c \
	fm_mso_cust_search_bill.c \
	fm_mso_cust_modify_data_access.c \
	fm_mso_cust_bb_get_info.c \
	fm_mso_cust_create_bb_service.c \
	fm_mso_cust_manage_plans_activation.c \
	fm_mso_cust_allocate_hardware.c \
	fm_mso_cust_activate_bb_service.c \
	fm_mso_cust_hold_service.c \
	fm_mso_cust_unhold_service.c \
	fm_mso_cust_price_calculator.c \
	fm_mso_cust_modify_schedule.c \
    	fm_mso_cust_get_plan_rates.c  \
	fm_mso_cust_view_invoice.c \
	fm_mso_cust_extend_validity.c \
	fm_mso_cust_topup_bb_plan.c \
	fm_mso_cust_renew_bb_plan.c \
	fm_mso_cust_swap_bb_device.c \
	fm_mso_cust_deactivate_bb_service.c \
	fm_mso_cust_delete_bb_device.c \
	fm_mso_cust_bb_hw_amc.c \
        fm_mso_cust_blk_register_customer.c\
	fm_mso_cust_dvbip_register_customer.c\
	fm_mso_cust_get_higher_arpu_plans.c \
	fm_mso_cust_get_catv_refund.c \
	fm_mso_cust_catv_swap_srvc_tagging.c \
	fm_mso_cust_catv_transfer_subscription.c \
	fm_mso_cust_create_offers.c \
	fm_mso_cust_update_gst_info.c \
	fm_mso_cust_catv_discount_config.c \
	fm_mso_cust_device_authenticate.c \
	fm_mso_cust_create_promo_obj.c \
	fm_mso_cust_device_authenticate.c \
	fm_mso_cust_netflix_register.c \
        fm_mso_cust_netflix_renew.c \
        fm_mso_cust_netflix_activate.c \
	fm_mso_cust_device_register.c \
	fm_mso_cust_apply_wallet_bal.c \
	fm_mso_cust_get_bouquet_ids.c \
        fm_mso_cust_get_lco_info.c \
        fm_mso_cust_hybrid_accts_tag.c \
        fm_mso_cust_get_pkg_channel_attr.c \
        fm_mso_cust_get_pkg_channels.c \
        fm_mso_cust_get_plan_details_list.c

OBJECTS= fm_mso_cust_config.o \
	fm_mso_cust_oap_login.o \
	fm_mso_cust_register_customer.o \
	fm_mso_cust_activate_customer.o \
	fm_mso_cust_get_plan_details.o \
	fm_mso_cust_get_customer_info.o \
	fm_mso_cust_set_customer_bit.o \
	fm_mso_cust_modify_customer_credentials.o \
	fm_mso_cust_modify_customer.o \
	fm_mso_cust_suspend_service.o \
        fm_mso_cust_reactivate_service.o \
        fm_mso_cust_terminate_service.o \
	fm_mso_cust_change_plan.o \
	fm_mso_cust_cancel_plan.o \
	fm_mso_cust_add_plan.o \
	fm_mso_cust_get_invoice.o\
	fm_mso_cust_create_schedule.o \
	fm_mso_cust_get_device_details.o \
	fm_mso_cust_get_channel_and_price.o \
	fm_mso_search.o \
	fm_mso_cust_search_bill.o \
	fm_mso_cust_modify_data_access.o \
	fm_mso_cust_bb_get_info.o \
	fm_mso_cust_create_bb_service.o \
	fm_mso_cust_manage_plans_activation.o \
	fm_mso_cust_allocate_hardware.o \
	fm_mso_cust_activate_bb_service.o \
	fm_mso_cust_hold_service.o \
	fm_mso_cust_unhold_service.o \
	fm_mso_cust_price_calculator.o \
	fm_mso_cust_modify_schedule.o \
    	fm_mso_cust_get_plan_rates.o  \
	fm_mso_cust_view_invoice.o \
	fm_mso_cust_extend_validity.o \
	fm_mso_cust_topup_bb_plan.o \
	fm_mso_cust_renew_bb_plan.o \
	fm_mso_cust_swap_bb_device.o \
	fm_mso_cust_deactivate_bb_service.o \
	fm_mso_cust_delete_bb_device.o \
	fm_mso_cust_bb_hw_amc.o \
        fm_mso_cust_blk_register_customer.o\
	fm_mso_cust_dvbip_register_customer.o\
	fm_mso_cust_get_higher_arpu_plans.o \
	fm_mso_cust_get_catv_refund.o \
	fm_mso_cust_catv_swap_srvc_tagging.o \
	fm_mso_cust_catv_transfer_subscription.o \
	fm_mso_cust_create_offers.o \
	fm_mso_cust_update_gst_info.o \
	fm_mso_cust_catv_discount_config.o \
	fm_mso_cust_device_authenticate.o \
	fm_mso_cust_create_promo_obj.o \
	fm_mso_cust_netflix_register.o \
        fm_mso_cust_netflix_renew.o \
        fm_mso_cust_netflix_activate.o \
	fm_mso_cust_device_register.o \
	fm_mso_cust_apply_wallet_bal.o \
	fm_mso_cust_get_bouquet_ids.o \
        fm_mso_cust_get_lco_info.o \
        fm_mso_cust_hybrid_accts_tag.o \
        fm_mso_cust_get_pkg_channel_attr.o \
        fm_mso_cust_get_pkg_channels.o \
        fm_mso_cust_get_plan_details_list.o

LIBACT=fm_mso_cust.$(SL_EXT)

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
