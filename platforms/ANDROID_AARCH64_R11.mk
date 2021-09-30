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
ANDROID_NDK_HOME = /Projects/android-ndk-r11b/
ANDROID_PLATFORM = ${ANDROID_NDK_HOME}/platforms/android-21/arch-arm64/
ANDROID_STL     =  ${ANDROID_NDK_HOME}/sources/cxx-stl/gnu-libstdc++/4.9
CHAINDIR        = ${ANDROID_NDK_HOME}/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/

PREFIX          = aarch64-linux-android-

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
INCLUDE_FLAGS  += -I${ANDROID_STL}/include -I${ANDROID_STL}/libs/arm64-v8a/include
CFLAGS         := -Os -ffunction-sections -fsigned-char -pie -fPIC
CFLAGS         += -march=armv8-a+crc+crypto+fp+simd -ftree-vectorize
CFLAGS         += --sysroot=${ANDROID_PLATFORM}

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
