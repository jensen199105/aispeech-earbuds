#
# select release type
#

all:
ifeq (${CONFIG_WAKEUP_NCHANNEL}, y)
ifeq (${CONFIG_LITE-FESPD_HEADSET}, y)
	-${MAKE} -C lite-fespd
endif

ifeq (${CONFIG_LITE-FESPD_MOBILE}, y)
	-${MAKE} -C lite-fespd
endif	

ifeq (${CONFIG_LITE-FESPD_HOME}, y)
	-${MAKE} -C lite-fespd
endif

ifeq (${CONFIG_LITE-FESPD-AEC}, y)
	-${MAKE} -C lite-fespd-aec
endif
endif

ifeq (${CONFIG_LITE-WAKEUP}, y)
	-${MAKE} -C lite-wakeup
endif

ifeq (${CONFIG_LITE-UDA}, y)
	-${MAKE} -C lite-uda
endif

ifeq (${CONFIG_LITE-SEVC}, y)
	-${MAKE} -C lite-sevc
endif

clean:
ifeq (${CONFIG_LITE-FESPD_HEADSET}, y)
	-${MAKE} -C lite-fespd clean
endif

ifeq (${CONFIG_LITE-FESPD_MOBILE}, y)
	-${MAKE} -C lite-fespd clean
endif

ifeq (${CONFIG_LITE-FESPD_HOME}, y)
	-${MAKE} -C lite-fespd clean
endif

ifeq (${CONFIG_LITE-WAKEUP}, y)
	-${MAKE} -C lite-wakeup clean
endif

ifeq (${CONFIG_LITE-FESPD-AEC}, y)
	-${MAKE} -C lite-fespd-aec clean
endif

ifeq (${CONFIG_LITE-UDA}, y)
	-${MAKE} -C lite-uda clean
endif

ifeq (${CONFIG_LITE-SEVC}, y)
	-${MAKE} -C lite-sevc clean
endif

