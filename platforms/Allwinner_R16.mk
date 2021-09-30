#
# Voice Signal Preprocess
# Copyright (C) 1991-2017 Nationalchip Co., Ltd
#
# Makefile: VSP DSP Makefile
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
export LITE-FESPD_ROOT
export RM
# Environments
#=================================================================================================#
LITE-FESPD_ROOT = ${shell pwd}
PREFIX          = arm-openwrt-linux-
CHAINDIR        = /Projects/toolchain-R16/bin/
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
CFLAGS         := -Os -ffunction-sections -fsigned-char -fpic -fPIC 
CFLAGS         += -fvisibility=hidden -ffast-math -fomit-frame-pointer
CFLAGS         += -march=armv7-a -mfloat-abi=hard -mfpu=neon 
CFLAGS         += --sysroot=${CHAINDIR}/../

CFLAGS         += $(INCLUDE_FLAGS)
FORBID_SWITCH=n
# Linker options 
#=================================================================================================#
LIB_DIR         = 
LIBS            =
LIB_FLAGS       = $(addprefix -L , $(LIB_DIR))
LDFLAGS         = $(CORE_FLAGS) -Wl,--gc-sections
#LDFLAGS         = $(CORE_FLAGS) -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs -Wl,-Map,xx.map
LDFLAGS        += $(LIB_FLAGS) $(LIBS) -lm
