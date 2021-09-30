#
# Just generate for test
#
platform :=$(shell echo ${TARGET_PLATFORM} | tr '[A-Z]' '[a-z]')
CFLAGS += -I.
CFLAGS += -fvisibility=hidden -ffunction-sections

CFLAGS  += -DFIXED_POINT=32  
CFLAGS  += -DDSP_PLATFORM -DAISPEECH_FIXED
ifeq (${CONFIG_SEVC_MULTI_CORE}, y)
CFLAGS += -DSEVC_MULTI_CORE
endif

ifeq (${PROFILE}, y)
CFLAGS += -pg -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
endif

LDFLAGS += -lm

TARGET_SRC := ${shell find . -name "*.c"}

TARGET_OBJS := ${patsubst %.c, %.o, ${TARGET_SRC}}
LIBLITE_SEVC := liblite_sevc_*_${platform}_export.a
#LIBLITE-SEVC := liblite_sevc_*_${platform}.a
#LIBLITE_SEVC := ${shell find . -name "liblite_sevc_*_${platform}_export.a"}
TARGET := lite_sevc_${platform}

all:${TARGET}

${TARGET}:${TARGET_SRC} ${ROOT}/Release/lite-sevc/${LIBLITE_SEVC}
ifneq ("$(TARGET_PLATFORM)","BES2300")
	${CC} ${CFLAGS} -o  $@ $^ ${LDFLAGS} 
	mv $@ ${ROOT}/Release/lite-sevc/
	#mv xx.map ${ROOT}/Release/lite-sevc/${TARGET}.map
endif
clean:
	-rm -fr ${TARGET_OBJS} ${ROOT}/Release/lite-fespd-sevc/${TARGET}
	-rm -fr ${ROOT}/Release/lite-fespd-sevc/*.map

