#
# Voice Signal Preprocess
# Copyright (C) 1991-2017 Nationalchip Co., Ltd
#
# Makefile
#
#=================================================================================#
# Toolchain Configurations

export AR
export CC
export LD
export STRIP
export OBJCOPY
export CFLAGS
export LDFLAGS
export LIBLITE-FESPD
export LIBLITE-FESPD_EXPORT
export TARGET1
export CORE_FLAGS
export SYS_FLAGS
export LITE-FESPD_ROOT
export RM
export mobile

#=================================================================================#
# import liteos settings   
LITEOS_SDK_PATH = /Projects/Hi3518EV300_MobileCam_SDK_V1.0.0.3
LITEOSTOPDIR 	= $(LITEOS_SDK_PATH)/osdrv/platform/liteos/liteos

CFLAGS += $(LITEOS_CFLAGS) \
	-I $(LITEOSTOPDIR)/lib/libc/include \
    	-I $(LITEOSTOPDIR)/net/lwip_sack/include

include $(LITEOSTOPDIR)/config.mk

CFLAGS += $(LITEOS_MACRO) $(LITEOS_CFLAGS) $(LITEOS_OSDRV_INCLUDE) $(LITEOS_USR_INCLUDE)    
CFLAGS += -D__HuaweiLite__
CFLAGS += -nostdinc

# Environments
#=================================================================================================#
LITE-FESPD_ROOT = ${shell pwd}

CHAINDIR        = /Projects/arm-himix100-linux/bin/
PREFIX          = arm-himix100-linux-
AR              = ${CHAINDIR}${PREFIX}ar
CC              = ${CHAINDIR}${PREFIX}gcc
LD              = ${CHAINDIR}${PREFIX}ld
OBJCOPY         = ${CHAINDIR}${PREFIX}objcopy
OBJDUMP         = ${CHAINDIR}${PREFIX}objdump --disassemble
READELF         = ${CHAINDIR}${PREFIX}readelf -a
DUMPELF         = ${CHAINDIR}${PREFIX}dumpelf
STRIP           = ${CHAINDIR}${PREFIX}strip
GENLDSCRIPTS    = ${CHAINDIR}${PREFIX}genldscripts

RM              = rm -fr
SHELL           = /bin/bash

# Compiler options
#=================================================================================================#
INCLUDE_DIR     = 
INCLUDE_FLAGS   = $(addprefix -I , $(INCLUDE_DIR))

CFLAGS         += $(LITEOS_MACRO) $(LITEOS_CFLAGS) $(LITEOS_OSDRV_INCLUDE) $(LITEOS_USR_INCLUDE)
CFLAGS         += -D__HuaweiLite__
CFLAGS         += -nostdinc -nostdlib
CFLAGS         += -I${shell pwd}/third/include/
CFLAGS         += -fsigned-char -ffunction-sections -DAISPEECH_FIXED -DDSP_PLATFORM
CFLAGS         += -fPIC -fPIE -fstack-protector --param ssp-buffer-size=4
CFLAGS	       += -mcpu=arm926ej-s -mno-unaligned-access 
#CFLAGS        += -g -O0
CFLAGS         += -Os -fsigned-char
CFLAGS         += -ffunction-sections
CFLAGS         += -Werror -Wall
CFLAGS         += $(INCLUDE_FLAGS)

# Linker options 
#=================================================================================================#
LIB_DIR         = 
LIBS            =
LIB_FLAGS       = $(addprefix -L , $(LIB_DIR)) 
LDFLAGS        += $(CORE_FLAGS) -Wl,--gc-sections -Wl,-Map,xx.map
LDFLAGS        += $(LIB_FLAGS) $(LIBS) -lm -lstdc++ 
