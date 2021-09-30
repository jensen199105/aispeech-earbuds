#
# Just generate for test
#
CFLAGS += -I.

ifeq (${CONFIG_LITE-FESPD_HEADSET}, y)
CFLAGS += -DENABLE_LUDA_FOR_HEADSET
endif	
 
TARGET      := lite-uda
TARGET_SRC  := lite-uda.c resourceWB.c
TARGET_OBJS := ${patsubst %.c, %.o, ${TARGET_SRC}}
LIBLITE-UDA := liblite-uda_export.a

all:${TARGET}

${TARGET}:${TARGET_SRC}  ${ROOT}/Release/lite-uda/${LIBLITE-UDA}
	${CC} ${CFLAGS} -o  $@ $^ ${LDFLAGS} 
	mv $@     ${ROOT}/Release/lite-uda/
	mv xx.map ${ROOT}/Release/lite-uda/${TARGET}.map

clean:
	-rm -fr ${TARGET_OBJS} ${ROOT}/Release/lite-uda/${TARGET}
	-rm -fr ${ROOT}/Release/lite-uda/*.map

