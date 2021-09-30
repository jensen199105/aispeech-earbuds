#
# Just generate for lite-fespd-aec lib
#
LIBLITE-FESPD-AEC := liblite-fespd-aec.a
LIBLITE-FESPD-AEC_EXPORT := liblite-fespd-aec_export.a

LIBSRCS := ${shell find . -name "*.c"}

# 
# select different path based on scenes
#
ifeq (${CONFIG_LITE-FESPD_MOBILE}, y)
LIBSRCS += ${shell find ${ROOT}/algorithms/modules/uda/udalite_gsc/mobile  -name "*.c"}
CFLAGS  += -I${ROOT}/algorithms/modules/uda/udalite_gsc/mobile/include 
endif

ifeq (${CONFIG_LITE-FESPD_HEADSET}, y)
LIBSRCS += ${shell find ${ROOT}/algorithms/modules/uda/udalite_gsc/headset -name "*.c"}
CFLAGS += -I${ROOT}/algorithms/modules/uda/udalite_gsc/headset/include 
endif

ifeq (${CONFIG_LITE-FESPD_HOME}, y)
LIBSRCS += ${shell find ${ROOT}/algorithms/modules/uda/udalite_gsc/home    -name "*.c"}
CFLAGS += -I${ROOT}/algorithms/modules/uda/udalite_gsc/home/include 
endif

LIBSRCS += ${shell find ${ROOT}/algorithms/base/kissfft -name "*.c"}
LIBSRCS += ${shell find ${ROOT}/algorithms/base/pcm2wav -name "*.c"}
LIBSRCS += ${shell find ${ROOT}/algorithms/base/math -name "*.c"}
LIBSRCS += ${shell find ${ROOT}/algorithms/base/string -name "*.c"}

#
# lite-fespd-aec: support fixed version only!
#
#ifeq (${CONFIG_FLOATING_RELEASE}, y)
#LIBSRCS += ${shell find ${ROOT}/algorithms/base/rbin -name "*.c"}
#CFLAGS  += -I${ROOT}/algorithms/base/rbin
#else
CFLAGS  += -DSTATIC_MEM_FIXED_FFT -DGSC_FIXED 
CFLAGS  += -DFIXED_POINT=32
CFLAGS  += -DDSP_PLATFORM -DAISPEECH_FIXED 
#endif

LIBSRCS += ${shell find ${ROOT}/algorithms/modules/lite-wakeup/wtk/api -name "*.c"}
LIBSRCS += ${shell find ${ROOT}/algorithms/modules/lite-wakeup/wtk/core -name "*.c"}
ifeq (${CONFIG_WAKEUP_USE_VAD}, y)
LIBSRCS += ${shell find ${ROOT}/algorithms/modules/lite-wakeup/wtk/vad -name "*.c"}
endif
ifeq (${CONFIG_WAKEUP_NN_DNN}, y)
LIBSRCS += ${shell find ${ROOT}/algorithms/modules/lite-wakeup/wtk/vite -not -path "*/fsmn/*" -name "*.c"}
endif
ifeq (${CONFIG_WAKEUP_NN_FSMN}, y)
LIBSRCS += ${shell find ${ROOT}/algorithms/modules/lite-wakeup/wtk/vite -not -path "*/dnn/*" -name "*.c"}
endif
LIBSRCS += ${shell find ${ROOT}/algorithms/modules/lite-wakeup/wtk/wakeup -name "*.c"}
LIBSRCS += ${shell find ${ROOT}/algorithms/modules/aec -name "*.c"}
LIBOBJS := ${patsubst %.c, %.o, ${LIBSRCS}} 

# Definitions
#==========================================================================================#
#CFLAGS += -DAISP_TSL_INFO
CFLAGS += -DLFESPD_BUFFER_EN

-include config.mk

CFLAGS += -fvisibility=hidden -ffunction-sections
CFLAGS += -I${ROOT}/algorithms/base/include
CFLAGS += -I${ROOT}/algorithms/base/kissfft
CFLAGS += -I${ROOT}/algorithms/base/math
CFLAGS += -I${ROOT}/algorithms/base/string
CFLAGS += -I${ROOT}/algorithms/modules/lite-wakeup 
CFLAGS += -I${ROOT}/algorithms/modules/lite-wakeup/third/include
CFLAGS += -I${ROOT}/algorithms/modules/lite-wakeup/wtk/api
CFLAGS += -I${ROOT}/algorithms/modules/aec/include

CFLAGS += $(FORBID_CFLAGS)

LDFLAGS += -Wl,--gc-sections
all:${LIBLITE-FESPD-AEC}

${LIBLITE-FESPD-AEC}:${LIBOBJS}
	#echo ${LIBOBJS}
	${AR} -rcs $@ $^
	mv $@ ${ROOT}/Release/lite-fespd-aec/
	${LD} ${CORE_FLAGS} ${SYS_FLAGS} -r ${LIBOBJS} -o liblite-fespd-aec.o
	${OBJCOPY} ${CORE_FLAGS} --localize-hidden liblite-fespd-aec.o
	${AR} -rcs ${LIBLITE-FESPD-AEC_EXPORT} liblite-fespd-aec.o
	${RM} ${LIBOBJS} liblite-fespd-aec.o
	mv ${LIBLITE-FESPD-AEC_EXPORT} ${ROOT}/Release/lite-fespd-aec/
	-rm -f xx.map

clean:
	-rm -fr ${LIBOBJS} ${ROOT}/Release/lite-fespd-aec/${LIBLITE-FESPD-AEC_EXPORT} ${ROOT}/Release/lite-fespd-aec/${LIBLITE-FESPD-AEC}

