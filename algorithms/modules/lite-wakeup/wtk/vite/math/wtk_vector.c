#include "wtk_vector.h"

wtk_short_vector_t *wtk_short_vector_new(int size)
{
    wtk_short_vector_t *v;
    v = (wtk_short_vector_t *)wtk_calloc(1, wtk_vector_type_bytes(size, short));
    //v=(wtk_vector_t*)memalign(4096,wtk_vector_bytes(size));
    wtk_short_vector_init(v, size);
    return v;
}

