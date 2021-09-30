#ifndef WTK_STRING_WTK_STR_H_
#define WTK_STRING_WTK_STR_H_

#include "AISP_TSL_str.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wtk_string wtk_string_t;

struct wtk_string
{
    char *data;
    int len;
};

wtk_string_t *wtk_string_new(int len);
int wtk_string_delete(wtk_string_t *s);
wtk_string_t *wtk_string_dup_data2(char *data, int len);


#ifdef __cplusplus
};
#endif

#endif

