
# release specific
#=================================================================================================#
export ROOT
ROOT := $(shell pwd)
export FORBID_SWITCH

include lite-fesp.mk
$(shell `.githooks/githook_setup.sh`)
$(shell `./resource.sh`)




# Definitions used to forbid use std calls
#==========================================================================================#
export FORBID_CFLAGS
FORBID_CFLAGS  := -Dmemset=__memset_is_forbidden
FORBID_CFLAGS  += -Dmemcpy=__memcpy_is_forbidden
FORBID_CFLAGS  += -Dmemmove=__memmove_is_forbidden
FORBID_CFLAGS  += -Dmemcmp=__memcmp_is_forbidden
FORBID_CFLAGS  += -Dstrlen=__strlen_is_forbidden
FORBID_CFLAGS  += -Dstrtof=__strtof_is_forbidden
FORBID_CFLAGS  += -Dstrcat=__strcat_is_forbidden
FORBID_CFLAGS  += -Dstrncat=__strncat_is_forbidden
FORBID_CFLAGS  += -Dstrcpy=__strcpy_is_forbidden
FORBID_CFLAGS  += -Dstrncpy=__strncpy_is_forbidden
FORBID_CFLAGS  += -Dstrcmp=__strcmp_is_forbidden
FORBID_CFLAGS  += -Dstrncmp=__strncmp_is_forbidden
FORBID_CFLAGS  += -Dstrchr=__strchr_is_forbidden
FORBID_CFLAGS  += -Dstrstr=__strstr_is_forbidden
FORBID_CFLAGS  += -Dstrtok=__strtok_is_forbidden
FORBID_CFLAGS  += -Dstrtok_r=__strtok_r_is_forbidden
FORBID_CFLAGS  += -Ditoa=__itoa_is_forbidden
FORBID_CFLAGS  += -Datoi=__atoi_is_forbidden
FORBID_CFLAGS  += -Disspace=__isspace_is_forbidden
FORBID_CFLAGS  += -Dsprintf=__sprintf_is_forbidden
FORBID_CFLAGS  += -Dsnprintf=__snprintf_is_forbidden
FORBID_CFLAGS  += -Dvsprintf=__vsprintf_is_forbidden
FORBID_CFLAGS  += -Dvsnprintf=__vsnprintf_is_forbidden

# Targets 
#=================================================================================================#
TARGETS :=
LIBS	:= 

# Build Rules
#=================================================================================================#
.PHONY: clean all

all:
	-${MAKE} -C apps
	-${MAKE} -C examples

clean:
	-${MAKE} -C apps clean
	-${MAKE} -C examples clean
	-${RM} examples/lite-wakeup/resource.c
	-${RM} examples/lite-uda/resourceWB.c
