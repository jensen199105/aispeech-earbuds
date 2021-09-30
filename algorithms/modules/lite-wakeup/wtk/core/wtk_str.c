#ifdef AISP_TSL_INFO
#include <stdio.h>
#endif

#include "AISP_TSL_str.h"
#include "AISP_TSL_common.h"
#include "wtk/core/wtk_alloc.h"
#include "wtk_str.h"

wtk_string_t *wtk_string_new(int len)
{
    wtk_string_t *s;
    s = (wtk_string_t *)wtk_malloc(len + sizeof(*s));
    s->len = len;

    if (len > 0)
    {
        s->data = (char *)(s + 1);
    }
    else
    {
        s->data = 0;
    }

    return s;
}

int wtk_string_delete(wtk_string_t *s)
{
    wtk_free(s);
    return 0;
}

wtk_string_t *wtk_string_dup_data2(char *data, int len)
{
    wtk_string_t *s;
    s = wtk_string_new(len + 1);

    if (s && data)
    {
        AISP_TSL_memcpy(s->data, data, len);
    }

    s->data[len] = 0;
    return s;
}

