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
TC_DIR          = /Projects/arm-cortex_m4/gcc-arm-none-eabi-7-2017-q4-major/bin/
PREFIX          = arm-none-eabi-
AR              = ${TC_DIR}${PREFIX}ar
CC              = ${TC_DIR}${PREFIX}gcc
LD              = ${TC_DIR}${PREFIX}ld
OBJCOPY         = ${TC_DIR}${PREFIX}objcopy
OBJDUMP         = ${TC_DIR}${PREFIX}objdump --disassemble
READELF         = ${TC_DIR}${PREFIX}readelf -a
DUMPELF         = ${TC_DIR}${PREFIX}dumpelf
STRIP           = ${TC_DIR}${PREFIX}strip
GENLDSCRIPTS    = ${TC_DIR}${PREFIX}genldscripts
RM              = rm -fr
SHELL           = /bin/bash
# Compiler options
#=================================================================================================#
INCLUDE_DIR     = 
INCLUDE_FLAGS   = $(addprefix -I , $(INCLUDE_DIR))
CFLAGS         := -Os -fsigned-char
CFLAGS         += -ffunction-sections -fdata-sections -fno-builtin-printf -fverbose-asm
#CFLAGS         += -Werror -Wall
CFLAGS         += -mcpu=cortex-m4 -mthumb -mthumb-interwork -specs=nano.specs -specs=nosys.specs -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -std=gnu89
CFLAGS         += $(INCLUDE_FLAGS)

# Linker options 
#=================================================================================================#
LIB_DIR         = 
LIBS            =
LIB_FLAGS       = $(addprefix -L , $(LIB_DIR))
LDFLAGS         = $(CORE_FLAGS) -Wl,--gc-sections -Wl,-Map,xx.map 
LDFLAGS        += $(LIB_FLAGS) $(LIBS)
SYS_FLAGS       =
