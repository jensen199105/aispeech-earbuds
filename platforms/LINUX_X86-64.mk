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
export SYS_FLAGS
export LITE-FESPD_ROOT
export RM
export mobile

# Environments
#=================================================================================================#
LITE-FESPD_ROOT = ${shell pwd}
AR              = ar
CC              = gcc
LD              = ld
OBJCOPY         = objcopy
OBJDUMP         = objdump --disassemble
READELF         = readelf -a
DUMPELF         = dumpelf
STRIP           = strip
GENLDSCRIPTS    = genldscripts
RM              = rm -fr
SHELL           = /bin/bash

# Compiler options
#=================================================================================================#
INCLUDE_DIR     = 
INCLUDE_FLAGS   = $(addprefix -I , $(INCLUDE_DIR))

# CFLAGS         := -g -O0
CFLAGS         := -Os -fsigned-char 
CFLAGS         += -std=gnu89 -ffunction-sections -fdata-sections -fPIC
CFLAGS         += -Werror -Wall
CFLAGS         += $(INCLUDE_FLAGS)

# Linker options 
#=================================================================================================#
LIB_DIR         = 
LIBS            =
LIB_FLAGS       = $(addprefix -L , $(LIB_DIR))
LDFLAGS         = $(CORE_FLAGS) -Wl,--gc-sections -Wl,-Map,xx.map 
LDFLAGS        += $(LIB_FLAGS) $(LIBS) -lm
SYS_FLAGS       = #-m elf_i386 
