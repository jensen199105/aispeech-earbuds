#
#
#

#
# load global settings
#
-include config.mk
export TARGET_PLATFORM
#
# parse target platform
#
ifeq (${CONFIG_LITE-FESP_FOR_LINUX}, y)
TARGET_PLATFORM := LINUX
endif

ifeq (${CONFIG_LITE-FESP_FOR_LINUX_64}, y)
TARGET_PLATFORM := LINUX_X86-64
endif

ifeq (${CONFIG_LITE-FESP_FOR_HIFI3}, y)
TARGET_PLATFORM := HIFI3
endif

ifeq (${CONFIG_LITE-FESP_FOR_HIFI4}, y)
TARGET_PLATFORM := HIFI4
endif

ifeq (${CONFIG_LITE-FESP_FOR_VIVO-M4F}, y)
TARGET_PLATFORM := VIVO-M4F
endif

ifeq (${CONFIG_LITE-FESP_FOR_XR872-M4F}, y)
TARGET_PLATFORM := XR872-M4F
endif

ifeq (${CONFIG_LITE-FESP_FOR_MSTAR8339}, y)
TARGET_PLATFORM := MSTAR8339
endif

ifeq (${CONFIG_LITE-FESP_FOR_MSTAR848_M4}, y)
TARGET_PLATFORM := MSTAR848_M4
endif

ifeq (${CONFIG_LITE-FESP_FOR_HIMIX3518}, y)
TARGET_PLATFORM := HIMIX3518
endif

ifeq (${CONFIG_LITE-FESP_FOR_LITEOS_HISIV500}, y)
TARGET_PLATFORM := LITEOS-HISIV500
endif

ifeq (${CONFIG_LITE-FESP_FOR_LINUX_HI3556}, y)
TARGET_PLATFORM := LINUX-HI3556
endif

ifeq (${CONFIG_LITE-FESP_FOR_LINUX_HI3516E}, y)
TARGET_PLATFORM := LINUX-HI3516E
endif

ifeq (${CONFIG_LITE-FESP_FOR_RK3308_64}, y)
TARGET_PLATFORM := RK3308_32
endif

ifeq (${CONFIG_LITE-FESP_FOR_RK3308_64}, y)
TARGET_PLATFORM := RK3308_64
endif

ifeq (${CONFIG_LITE-FESP_FOR_MT8516_64}, y)
TARGET_PLATFORM := MT8516_64
endif

ifeq (${CONFIG_LITE-FESP_FOR_Allwinner_R16}, y)
TARGET_PLATFORM := Allwinner_R16
endif

ifeq (${CONFIG_LITE-FESP_FOR_MTK2712_64}, y)
TARGET_PLATFORM := MTK2712_64
endif

ifeq (${CONFIG_LITE-FESP_FOR_ANDROID_ARM_R11}, y)
TARGET_PLATFORM := ANDROID_ARM_R11
endif

ifeq (${CONFIG_LITE-FESP_FOR_ANDROID_AARCH64_R11}, y)
TARGET_PLATFORM := ANDROID_AARCH64_R11
endif

ifeq (${CONFIG_LITE-FESP_FOR_ANDROID_ARM_R14}, y)
TARGET_PLATFORM := ANDROID_ARM_R14
endif

ifeq (${CONFIG_LITE-FESP_FOR_ANDROID_AARCH64_R14}, y)
TARGET_PLATFORM := ANDROID_AARCH64_R14
endif

ifeq (${CONFIG_LITE-FESP_FOR_BES2300}, y)
TARGET_PLATFORM := BES2300
endif


#
# load detail specific parameters
#
-include platforms/${TARGET_PLATFORM}.mk
