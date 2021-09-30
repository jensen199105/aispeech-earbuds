#
# Just generate for lite-sevc lib
#

SEVC_VERSION := V2.0.0
platform :=$(shell echo ${TARGET_PLATFORM} | tr '[A-Z]' '[a-z]')
sevc_version := $(shell echo ${SEVC_VERSION} | tr '[A-Z]' '[a-z]')
LIBLITE-SEVC := liblite_sevc_${sevc_version}_${platform}.a
LIBLITE-SEVC_EXPORT := liblite_sevc_${sevc_version}_${platform}_export.a
LIBLITE-SEVC_SO := liblite_sevc_${sevc_version}_${platform}.so

GIT_COMMIT_ID := $(shell git rev-parse --short HEAD)
GIT_BRANCH := $(shell git symbolic-ref --short -q HEAD)
GIT_INFO := ${GIT_BRANCH}_$(GIT_COMMIT_ID)

#
# lite-sevc-aec: support fixed version only!
#
#ifeq (${CONFIG_FLOATING_RELEASE}, y)
#LIBSRCS += ${shell find ${ROOT}/algorithms/base/rbin -name "*.c"}
#CFLAGS  += -I${ROOT}/algorithms/base/rbin
#else
CFLAGS  += -DSTATIC_MEM_FIXED_FFT 
CFLAGS  += -DFIXED_POINT=32
CFLAGS  += -DDSP_PLATFORM -DAISPEECH_FIXED 
#endif

ifeq (${CONFIG_FFT_256}, y)
CFLAGS += -DFFT_SIZE_256
endif

ifeq (${CONFIG_SEVC_PHONE}, y)
CFLAGS += -DSEVC_PHONE
endif
ifeq (${CONFIG_SEVC_CAR}, y)
CFLAGS += -DSEVC_CAR
endif

ifeq (${CONFIG_FAR_FIELD_COMMUNICATION}, y)
CFLAGS += -DFAR_FIELD_COMMUNICATION
endif

ifeq (${CONFIG_SEVC_MULTI_CORE}, y)
CFLAGS += -DSEVC_MULTI_CORE
else
ifeq (${CONFIG_USE_SHARE_MEM}, y)
CFLAGS += -DUSE_SHARE_MEM
endif
endif

LIBSRCS := ${shell find . -name "*.c"}
# 
# select different path based on scenes
#

LIBSRCS += ${shell find ${ROOT}/algorithms/base/kissfft -name "*.c"}
LIBSRCS += ${shell find ${ROOT}/algorithms/base/math -name "*.c"}
LIBSRCS += ${shell find ${ROOT}/algorithms/base/string -name "*.c"}
#LIBSRCS += ${shell find ${ROOT}/algorithms/base/mem -name "*.c"}
#LIBSRCS += ${shell find ${ROOT}/algorithms/base/rbin -name "*.c"}
#LIBSRCS += ${shell find ${ROOT}/algorithms/base/json -name "*.c"}

#LIBSRCS += ${shell find ${ROOT}/algorithms/modules/lite-sevc/src -name "*.c"}
LIBSRCS += ${wildcard ${ROOT}/algorithms/modules/sevc/src/*.c}
LIBSRCS += ${wildcard ${ROOT}/algorithms/modules/sevc/src/model/utils/*.c}
ifeq (${CONFIG_SEVC_NN_GRU}, y)
CFLAGS += -I${ROOT}/algorithms/modules/sevc/src/model/gru
LIBSRCS += ${shell find ${ROOT}/algorithms/modules/sevc/src/model/gru -name "*.c"}
endif

ifeq (${CONFIG_SEVC_NN_FSMN}, y)
CFLAGS += -DNN_FSMN
CFLAGS += -I${ROOT}/algorithms/modules/sevc/src/model/fsmn
LIBSRCS += ${shell find ${ROOT}/algorithms/modules/sevc/src/model/fsmn -name "*.c"}
endif

LIBOBJS := ${patsubst %.c, %.o, ${LIBSRCS}} 

# Definitions
#==========================================================================================#
CFLAGS += -DAISP_TSL_INFO

-include config.mk

CFLAGS += -fvisibility=hidden -ffunction-sections
CFLAGS += -I${ROOT}/algorithms/base/include
CFLAGS += -I${ROOT}/algorithms/base/kissfft
CFLAGS += -I${ROOT}/algorithms/base/math
CFLAGS += -I${ROOT}/algorithms/base/string
#CFLAGS += -I${ROOT}/algorithms/base/mem
CFLAGS += -I${ROOT}/algorithms/base/rbin
CFLAGS += -I${ROOT}/algorithms/base/json
CFLAGS += -I${ROOT}/algorithms/modules/sevc/include
CFLAGS += -I${ROOT}/algorithms/modules/sevc/src/model/utils
CFLAGS += -I${ROOT}/algorithms/modules/sevc/resource/nn_resource
CFLAGS += -I${ROOT}/algorithms/modules/sevc/resource/bf_resource

ifneq (${FORBID_SWITCH}, n)
CFLAGS += $(FORBID_CFLAGS)
endif
CFLAGS += -Wall -fPIC

LDFLAGS += -Wl,--gc-sections
#all:prepare ${LIBLITE-SEVC_SO} ${LIBLITE-SEVC}
all:prepare ${LIBLITE-SEVC}

prepare: VERSION_INFO


VERSION_INFO:
	python ${ROOT}/tools/version_generate.py ${ROOT}/algorithms/modules/sevc/include sevc ${SEVC_VERSION} ${GIT_INFO} 

${LIBLITE-SEVC}:${LIBOBJS}
	${AR} -rcs $@ $^
	mkdir -p ${ROOT}/Release/lite-sevc
	mv $@ ${ROOT}/Release/lite-sevc/
	${LD} ${CORE_FLAGS} ${SYS_FLAGS} -r ${LIBOBJS} -o liblite-sevc.o
	${OBJCOPY} ${CORE_FLAGS} --localize-hidden liblite-sevc.o
	${AR} -rcs ${LIBLITE-SEVC_EXPORT} liblite-sevc.o
	${RM} ${LIBOBJS} liblite-sevc.o
	mv ${LIBLITE-SEVC_EXPORT} ${ROOT}/Release/lite-sevc/
	-rm -f xx.map

${LIBLITE-SEVC_SO}:${LIBOBJS}
	$(CC) -shared -fPIC -o $@ ${CFLAGS} ${LIBOBJS}
	mkdir -p ${ROOT}/Release/lite-sevc
	mv ${LIBLITE-SEVC_SO} ${ROOT}/Release/lite-sevc/

clean:
	#-rm -fr ${LIBOBJS} ${ROOT}/Release/lite-sevc/${LIBLITE-SEVC_EXPORT} ${ROOT}/Release/lite-sevc/${LIBLITE-SEVC}
	-rm -fr ${LIBOBJS} ${ROOT}/Release/lite-sevc/*${platform}*

