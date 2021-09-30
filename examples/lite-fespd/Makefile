#
# Just generate for test
#
CFLAGS += -I.

# The output data is taken from gscProcessMobile(...) directly, not from dataPop(...).
ifeq (${CONFIG_LITE-FESPD_FREQOUT_DIRECT}, y)
CFLAGS += -DLUDA_SUPPORT_FREQOUT_DIRECT
endif

TARGET      := lite-fespd
TARGET_SRC  := lite-fespd.c ../lite-wakeup/resource.c ../lite-uda/resourceWB.c
TARGET_OBJS := ${patsubst %.c, %.o, ${TARGET_SRC}}
LIBLITE-FESPD := liblite-fespd_export.a

all:${TARGET}

${TARGET}:${TARGET_SRC}  ${ROOT}/Release/lite-fespd/${LIBLITE-FESPD}
	${CC} ${CFLAGS} -o  $@ $^ ${LDFLAGS} 
	mv $@ ${ROOT}/Release/lite-fespd/
	mv xx.map ${ROOT}/Release/lite-fespd/${TARGET}.map

clean:
	-rm -fr ${TARGET_OBJS} ${ROOT}/Release/lite-fespd/${TARGET}
	-rm -fr ${ROOT}/Release/lite-fespd/*.map

