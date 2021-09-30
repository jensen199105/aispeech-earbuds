#
# Creating libwakeup.a
#

PWD := ${shell pwd}
CFLAGS += -I${PWD}/../ -DUSE_DUMMY_ICON -fvisibility=hidden

ifeq (${use_vad}, n)
SRCs   := ${shell find . -path ./vad -prune -o -type f -name "*.c" -print}
else
SRCs   := ${shell find . -name "*.c"}
endif

OBJs   := ${patsubst %.c, %.o, ${SRCs}}

all:${LIBWAKEUP}

${LIBWAKEUP}:${OBJs}
	${AR} -rcs ../libaispeech_wakeup.a ${OBJs}
ifeq ($(SHARED_LIB_EN), y)
	${LD} ${PLATFORM_LD_FLAGS} -shared -o ../libaispeech_wakeup.so ${OBJs}
endif

ifeq ($(CC), xt-xcc)
	${LD} ${CORE_FLAGS} -r ${OBJs} -o libwakeup.o
	${OBJCOPY} ${CORE_FLAGS} --localize-hidden libwakeup.o
	${AR} -rcs ${LIBWAKEUP} libwakeup.o
	mv ${LIBWAKEUP} ../
	rm -f libwakeup.o
else
ifeq ("${PLATFORM}", "MIPS")
	${LD} ${CORE_FLAGS} -r ${OBJs} -o libwakeup.o
	${OBJCOPY} ${CORE_FLAGS} libwakeup.o
	${AR} -rcs ${LIBWAKEUP} libwakeup.o
	mv ${LIBWAKEUP} ../
	rm -f libwakeup.o
else
	${LD} ${PLATFORM_LD_FLAGS} -r ${OBJs} -o libwakeup.o
	${OBJCOPY} --localize-hidden libwakeup.o
	${AR} -rcs ${LIBWAKEUP} libwakeup.o
	mv ${LIBWAKEUP} ../
	rm -f libwakeup.o
endif
endif

clean:
	-rm -fr ${OBJs} ../libaispeech_wakeup.a ../libaispeech_wakeup.so ../${LIBWAKEUP}
