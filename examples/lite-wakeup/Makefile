#
# Just generate for test
#
CFLAGS += -I.

WKP_BIN   := lite-wakeup
WKP_SRCS  := lwakeup.c resource.c
WKP_OBJS  := ${patsubst %.c, %.o, ${WKP_SRCS}}

NWKP_BIN  := lite-nwakeup
NWKP_SRCS := lnwakeup.c resource.c
NWKP_OBJS := ${patsubst %.c, %.o, ${NWKP_SRC}}

LIBLITE-WAKEUP := liblite-wakeup_export.a

ifeq (${CONFIG_WAKEUP_NCHANNEL}, y)
all: ${WKP_BIN} ${NWKP_BIN}
else
all: ${WKP_BIN}
endif

${WKP_BIN}: ${WKP_SRCS} ${ROOT}/Release/lite-wakeup/${LIBLITE-WAKEUP}
	${CC} ${CFLAGS} -o  $@ $^ ${LDFLAGS}
	mv $@ ${ROOT}/Release/lite-wakeup/
	mv xx.map ${ROOT}/Release/lite-wakeup/${@}.map

ifeq (${CONFIG_WAKEUP_NCHANNEL}, y)
${NWKP_BIN}: ${NWKP_SRCS} ${ROOT}/Release/lite-wakeup/${LIBLITE-WAKEUP}
	${CC} ${CFLAGS} -o  $@ $^ ${LDFLAGS}
	mv $@ ${ROOT}/Release/lite-wakeup/
	mv xx.map ${ROOT}/Release/lite-wakeup/${@}.map
endif

clean:
	-rm -fr ${WKP_OBJS} ${NWKP_OBJS} ${ROOT}/Release/lite-wakeup/${WKP_BIN} ${ROOT}/Release/lite-wakeup/${NWKP_BIN}
	-rm -fr ${ROOT}/Release/lite-wakeup/*.map

