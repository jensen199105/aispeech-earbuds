#
# Just generate for lite-uda lib
#
LIBLITE-UDA        := liblite-uda.a
LIBLITE-UDA_EXPORT := liblite-uda_export.a

#
# compile lite-uda.c 
#
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

#
# common source files
#
LIBSRCS += ${shell find ${ROOT}/algorithms/base/kissfft -name "*.c"}
LIBSRCS += ${shell find ${ROOT}/algorithms/base/pcm2wav -name "*.c"}
LIBSRCS += ${shell find ${ROOT}/algorithms/base/math    -name "*.c"}
LIBSRCS += ${shell find ${ROOT}/algorithms/base/string  -name "*.c"}

ifeq (${CONFIG_FLOATING_RELEASE}, y)
LIBSRCS += ${shell find ${ROOT}/algorithms/base/rbin -name "*.c"}
CFLAGS  += -I${ROOT}/algorithms/base/rbin
else
CFLAGS  += -DSTATIC_MEM_FIXED_FFT -DGSC_FIXED 
CFLAGS  += -DFIXED_POINT=32
CFLAGS  += -DDSP_PLATFORM -DAISPEECH_FIXED 
endif

LIBOBJS := ${patsubst %.c, %.o, ${LIBSRCS}} 

# Definitions
#==========================================================================================#

-include config.mk

CFLAGS += -fvisibility=hidden -ffunction-sections
CFLAGS += -I${ROOT}/algorithms/base/include
CFLAGS += -I${ROOT}/algorithms/base/kissfft
CFLAGS += -I${ROOT}/algorithms/base/math
CFLAGS += -I${ROOT}/algorithms/base/string

CFLAGS += $(FORBID_CFLAGS)

LDFLAGS += -Wl,--gc-sections

all:${LIBLITE-UDA}

${LIBLITE-UDA}:${LIBOBJS}
	${AR} -rcs $@ $^
	mv $@ ${ROOT}/Release/lite-uda/
	${LD} ${CORE_FLAGS} ${SYS_FLAGS} -r ${LIBOBJS} -o liblite-uda.o
	${OBJCOPY} ${CORE_FLAGS} --localize-hidden liblite-uda.o
	${AR} -rcs ${LIBLITE-UDA_EXPORT} liblite-uda.o
	${RM} ${LIBOBJS} liblite-uda.o
	mv ${LIBLITE-UDA_EXPORT} ${ROOT}/Release/lite-uda/
	-rm -f xx.map

clean:
	-rm -fr ${LIBOBJS} ${ROOT}/Release/lite-uda/${LIBLITE-UDA_EXPORT} ${ROOT}/Release/lite-uda/${LIBLITE-UDA}

