#ifndef WTK_CORE_WTK_TYPE_H_
#define WTK_CORE_WTK_TYPE_H_

#include <stdint.h>

#include "AISP_TSL_types.h"
#include "wtk_alloc.h"

#define wtk_debug(...)

#define data_offset(q, type, link) (type *)((q) ? (void *)((char *)q - offsetof(type, link)) : NULL)

#endif

