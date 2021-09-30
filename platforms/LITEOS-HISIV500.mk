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
LITEOS_SDK_PATH = /Projects/Hi3516CV300_SDK_V1.0.1.1
LITEOSTOPDIR 	= $(LITEOS_SDK_PATH)/osdrv/opensource/liteos/liteos
include $(LITEOSTOPDIR)/config.mk


# Environments
#=================================================================================================#
LITE-FESPD_ROOT = ${shell pwd}
TC_DIR          = /Projects/hisi-linux/x86-arm/arm-hisiv500-linux/bin/
PREFIX          = arm-hisiv500-linux-uclibcgnueabi-
AR              = ${TC_DIR}${PREFIX}ar
CC              = ${TC_DIR}${PREFIX}gcc
LD              = ${TC_DIR}${PREFIX}ld
OBJCOPY         = ${TC_DIR}${PREFIX}objcopy
OBJDUMP         = ${TC_DIR}${PREFIX}objdump --disassemble
READELF         = ${TC_DIR}${PREFIX}readelf -a
DUMPELF         = ${TC_DIR}${PREFIX}dumpelf
STRIP           = ${TC_DIR}${PREFIX}strip
GENLDSCRIPTS    = genldscripts
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
CFLAGS	       += -mcpu=arm926ej-s -mno-unaligned-access -mlong-calls
#CFLAGS        += -g -O0
CFLAGS         += -Os -fsigned-char
CFLAGS         += -ffunction-sections #-m32
CFLAGS         += -Werror -Wall
CFLAGS         += $(INCLUDE_FLAGS)


# Linker options 
#=================================================================================================#
LIB_DIR         =
LIBS            =
LIB_FLAGS       = $(addprefix -L , $(LIB_DIR))
LDFLAGS         = $(CORE_FLAGS) -Wl,--gc-sections -Wl,-Map,xx.map
LDFLAGS        += $(LIB_FLAGS) $(LIBS)
SYS_FLAGS       =

