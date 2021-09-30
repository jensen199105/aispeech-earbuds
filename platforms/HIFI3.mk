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
XTENSA          = /opt/xtensa
CHAINDIR        = ${XTENSA}/XtDevTools/install/tools/RG-2018.9-linux/XtensaTools/bin
AR              = ${CHAINDIR}/xt-ar
CC              = ${CHAINDIR}/xt-xcc
LD              = ${CHAINDIR}/xt-ld
OBJCOPY         = ${CHAINDIR}/xt-objcopy
OBJDUMP         = ${CHAINDIR}/xt-objdump --disassemble
READELF         = ${CHAINDIR}/xt-readelf -a
DUMPELF         = ${CHAINDIR}/xt-dumpelf
STRIP           = ${CHAINDIR}/xt-strip
GENLDSCRIPTS    = ${CHAINDIR}/xt-genldscripts
XTENSA_CORE     = hifi3_bd5
XPLORER_VERSION = RG-2018.9-linux
XTENSA_SYSTEM   = $(XTENSA)/XtDevTools/install/builds/$(XPLORER_VERSION)/$(XTENSA_CORE)/config
CORE_FLAGS      = --xtensa-system=$(XTENSA_SYSTEM) --xtensa-core=$(XTENSA_CORE) 
RM              = rm -fr
SHELL           = /bin/bash

# Compiler options
#=================================================================================================#
INCLUDE_DIR     =
INCLUDE_FLAGS   = $(addprefix -I , $(INCLUDE_DIR))
CFLAGS         := -O3 -ffunction-sections -fsigned-char
#CFLAGS         += -Werror -Wall
CFLAGS         += $(CORE_FLAGS) -mcoproc 
CFLAGS         += -LNO:simd -LNO:simd_agg_if_conv -LNO:simd_v -fmessage-length=0
CFLAGS         += $(INCLUDE_FLAGS)

# Linker options 
#=================================================================================================#
LIB_DIR         = 
LIBS            =
LIB_FLAGS       = $(addprefix -L , $(LIB_DIR))
LDFLAGS         = $(CORE_FLAGS) -Wl,--gc-sections -Wl,-Map,xx.map  
LDFLAGS        += $(LIB_FLAGS) $(LIBS) -lm