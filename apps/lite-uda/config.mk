#
# config for scene, such as mobile, headset, home
#
ifeq (${CONFIG_LITE-FESPD_MOBILE}, y)
CFLAGS += -DENABLE_LUDA_FOR_MOBILE
endif

ifeq (${CONFIG_LITE-FESPD_HEADSET}, y)
CFLAGS += -DENABLE_LUDA_FOR_HEADSET
endif

ifeq (${CONFIG_LITE-FESPD_HOME}, y)
CFLAGS += -DENABLE_LUDA_FOR_HOME
endif

#
# config for fft length, 256 or 512
#
ifeq (${CONFIG_RDMA_FFT_256}, y)
CFLAGS  += -DRDMA_FFT_256
else
CFLAGS  += -DRDMA_FFT_512
endif

#
# other configs
#
ifeq (${CONFIG_GSC_HALF_VAD}, y)
CFLAGS += -DGSC_ENABLE_HALF_VAD
endif

ifeq (${CONFIG_FLOATING_RELEASE}, y)
CFLAGS += -DFLOATING_RELEASE
endif

ifeq (${CONFIG_PLATFORM_LE}, y)
CFLAGS += -D__PLATFORM_LE__
endif

