#
# Just generate for lite-fespd lib
#

-include config.mk

LIBLITE-WAKEUP := liblite-wakeup.a
LIBLITE-WAKEUP_EXPORT:= liblite-wakeup_export.a
LIBSRCS := ${shell find . -name "*.c"}
LIBSRCS += ${shell find ${LITE-FESPD_ROOT}/algorithms/base/kissfft -name "*.c"}
LIBSRCS += ${shell find ${LITE-FESPD_ROOT}/algorithms/base/math -name "*.c"}
LIBSRCS += ${shell find ${LITE-FESPD_ROOT}/algorithms/base/string -name "*.c"}
LIBSRCS += ${shell find ${LITE-FESPD_ROOT}/algorithms/modules/lite-wakeup/wtk/api -name "*.c"}
LIBSRCS += ${shell find ${LITE-FESPD_ROOT}/algorithms/modules/lite-wakeup/wtk/core -name "*.c"}
ifeq (${CONFIG_WAKEUP_USE_VAD}, y)
LIBSRCS += ${shell find ${LITE-FESPD_ROOT}/algorithms/modules/lite-wakeup/wtk/vad -name "*.c"}
endif
ifeq (${CONFIG_WAKEUP_NN_DNN}, y)
LIBSRCS += ${shell find ${LITE-FESPD_ROOT}/algorithms/modules/lite-wakeup/wtk/vite -not -path "*/fsmn/*" -name "*.c"}
endif
ifeq (${CONFIG_WAKEUP_NN_FSMN}, y)
LIBSRCS += ${shell find ${LITE-FESPD_ROOT}/algorithms/modules/lite-wakeup/wtk/vite -not -path "*/dnn/*" -name "*.c"}
endif
LIBSRCS += ${shell find ${LITE-FESPD_ROOT}/algorithms/modules/lite-wakeup/wtk/wakeup -name "*.c"}

LIBOBJS := ${patsubst %.c, %.o, ${LIBSRCS}} 

# Definitions
#==========================================================================================#
CFLAGS  += -DFIXED_POINT=32
CFLAGS  += -DSTATIC_MEM_FIXED_FFT
CFLAGS  += -DDSP_PLATFORM -DAISPEECH_FIXED
CFLAGS  += -DAISP_TSL_INFO
CFLAGS  += -DLWKP_API_EXPORT_EN

CFLAGS += -fvisibility=hidden -ffunction-sections
CFLAGS += -I${LITE-FESPD_ROOT}/apps/include
CFLAGS += -I${LITE-FESPD_ROOT}/algorithms/base/include
CFLAGS += -I${LITE-FESPD_ROOT}/algorithms/base/kissfft
CFLAGS += -I${LITE-FESPD_ROOT}/algorithms/base/math
CFLAGS += -I${LITE-FESPD_ROOT}/algorithms/base/string
CFLAGS += -I${LITE-FESPD_ROOT}/algorithms/modules/lite-wakeup 
CFLAGS += -I${LITE-FESPD_ROOT}/algorithms/modules/lite-wakeup/third/include
CFLAGS += -I${LITE-FESPD_ROOT}/algorithms/modules/lite-wakeup/wtk/api

CFLAGS += $(FORBID_CFLAGS)

LDFLAGS += -Wl,--gc-sections
all:${LIBLITE-WAKEUP}

${LIBLITE-WAKEUP}:${LIBOBJS}
	#echo ${LIBOBJS}
	${AR} -rcs $@ $^
	mv $@ ${ROOT}/Release/lite-wakeup/
	${LD} ${CORE_FLAGS} ${SYS_FLAGS} -r ${LIBOBJS} -o liblite-wakeup.o
	${OBJCOPY} ${CORE_FLAGS} --localize-hidden liblite-wakeup.o
	${AR} -rcs ${LIBLITE-WAKEUP_EXPORT} liblite-wakeup.o
	${RM} ${LIBOBJS} liblite-wakeup.o
	mv ${LIBLITE-WAKEUP_EXPORT} ${ROOT}/Release/lite-wakeup/
	-rm -f xx.map

clean:
	-rm -fr ${LIBOBJS} ${ROOT}/Release/lite-wakeup/${LIBLITE-WAKEUP_EXPORT} ${ROOT}/Release/lite-wakeup/${LIBLITE-WAKEUP}

