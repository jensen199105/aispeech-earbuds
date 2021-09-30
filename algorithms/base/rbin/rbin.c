/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : rbin.c
  Project    :
  Module     :
  Version    :
  Date       : 2017/06/19
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2017/06/19         1.00              Create

************************************************************/
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include "AISP_TSL_str.h"
#include "rbin.h"


#define RBIN2_MAX_ALLOC_FROM_HEAP  (4096 - 1)
#define rbin2_round(size, align) ((align) > 0 ? (((size) + ((align)-1)) & (~((align)-1))) : size)
#define rbin2_align_ptr(ptr, align)                                                                            \
    ((align) > 0 ? (void *)(((unsigned long)(ptr) + (align - 1)) & (~(align - 1))) : ptr)

#define data_offset(q, type, link) (type *)((q) ? (void *)((char *)q - offsetof(type, link)) : NULL)
#define rbin2_cfg_item_is_lc(cfg) ((cfg)->type==RBIN2_CFG_LC)
#define rbin2_cfg_item_is_string(cfg) ((cfg)->type==RBIN2_CFG_STRING)
#define rbin2_queue_init(q) memset(q,0,sizeof(*q))
#define rbin2_string_set(str, d, bytes)                                                                        \
    (str)->data = d;                                                                                         \
    (str)->len = bytes;
#define rbin2_string_set_s(str, data) rbin2_string_set(str, data, sizeof(data) - 1)
#define rbin2_str_equal(s1, ns1, s2, ns2) ((ns1 == ns2) && (AISP_TSL_strncmp(s1, s2, ns1) == 0))
#define rbin2_str_equal_s(s1, ns1, s2) rbin2_str_equal(s1, ns1, s2, sizeof(s2) - 1)
#define rbin2_strbuf_reset(b) ((b)->pos=0)
#define is_extchar(c) ((c)==':'||(c)=='.'||(c)=='\''||(c)=='!'||(c)=='-'||(c)=='_')
#define is_char(c) (isalnum(c)||is_extchar(c))
#define rbin2_malloc(n) calloc(1, n)
#define rbin2_calloc(nmem, size) calloc(nmem, size)
#define rbin2_free(p) free(p)
#define is_value(c) (isalnum((c))||(c==':')||c=='/'||c=='\\'||c=='_'||c=='-'||c=='.')
#define rbin2_cfg_queue_find_s(c,k) rbin2_cfg_queue_find(c,k,sizeof(k)-1)

int rbin2_cfg_file_feed_string(rbin2_cfg_file_t *c, char *d, int bytes);
int rbin2_cfg_file_process_include(rbin2_cfg_file_t *cfg);
rbin2_string_t *rbin2_local_cfg_find_string(rbin2_local_cfg_t *cfg, char *d, int bytes);
rbin2_string_t *rbin2_local_cfg_find_string2(rbin2_local_cfg_t *cfg, char *d, int bytes, int recursive);
void *rbin2_heap_malloc_large(rbin2_heap_t *heap, size_t size);
void *rbin2_heap_malloc(rbin2_heap_t *heap, size_t size);
rbin2_array_t *rbin2_array_new_h(rbin2_heap_t *h, uint32_t n, uint32_t size);
void *rbin2_array_push(rbin2_array_t *a);

int rbin2_string_cmp(rbin2_string_t *str, char *s, int bytes)
{
    int ret;
    ret = str->len - bytes;

    if (ret != 0)
    {
        goto end;
    }

    ret = AISP_TSL_strncmp(str->data, s, bytes);
end:
    return ret;
}

rbin2_cfg_item_t *rbin2_cfg_queue_find(rbin2_cfg_queue_t *c, char *k, int bytes)
{
    rbin2_cfg_item_t *i = 0, *p;
    rbin2_queue_node_t *n;

    for (n = c->queue.pop; n; n = n->next)
    {
        p = data_offset(n, rbin2_cfg_item_t, n);

        if (rbin2_string_cmp(p->key, k, bytes) == 0)
        {
            i = p;
            break;
        }
    }

    return i;
}

rbin2_local_cfg_t *rbin2_local_cfg_find_lc(rbin2_local_cfg_t *cfg, char *d, int bytes)
{
    rbin2_cfg_item_t *i;
    rbin2_local_cfg_t *lc = 0;

    if (!cfg)
    {
        goto end;
    }

    i = (rbin2_cfg_item_t *)rbin2_cfg_queue_find(cfg->cfg, d, bytes);

    if (i && rbin2_cfg_item_is_lc(i))
    {
        lc = i->value.cfg;
        goto end;
    }

    lc = rbin2_local_cfg_find_lc(cfg->parent, d, bytes);
end:
    return lc;
}

rbin2_string_t *rbin2_local_cfg_find_string(rbin2_local_cfg_t *cfg, char *d, int bytes)
{
    return rbin2_local_cfg_find_string2(cfg, d, bytes, 1);
}

rbin2_string_t *rbin2_local_cfg_find_string2(rbin2_local_cfg_t *cfg, char *d, int bytes, int recursive)
{
    rbin2_string_t *name = 0;
    rbin2_cfg_item_t *i;

    if (!cfg)
    {
        goto end;
    }

    i = (rbin2_cfg_item_t *)rbin2_cfg_queue_find(cfg->cfg, d, bytes);

    if (i && rbin2_cfg_item_is_string(i))
    {
        name = i->value.str;
        goto end;
    }

    if (recursive)
    {
        name = rbin2_local_cfg_find_string(cfg->parent, d, bytes);
    }

end:
    return name;
}

rbin2_array_t *rbin2_local_cfg_find_int_array(rbin2_local_cfg_t *cfg, char *d, int bytes)
{
    rbin2_array_t *a = 0, *b = 0;
    rbin2_cfg_item_t *i;
    rbin2_string_t **v;
    int k, j;

    if (!cfg)
    {
        goto end;
    }

    i = (rbin2_cfg_item_t *)rbin2_cfg_queue_find(cfg->cfg, d, bytes);

    if (i && rbin2_cfg_item_is_array(i))
    {
        b = i->value.array;
        goto end;
    }

    b = rbin2_local_cfg_find_array(cfg->parent, d, bytes);

    if (!b)
    {
        goto end;
    }

end:

    if (b)
    {
        v = (rbin2_string_t **)b->slot;
        a = rbin2_array_new_h(cfg->heap, b->nslot, sizeof(int));

        for (k = 0; k < b->nslot; ++k)
        {
            j = AISP_TSL_atoi(v[k]->data);
            *((int *)rbin2_array_push(a)) = j;
        }
    }

    return a;
}

rbin2_array_t *rbin2_local_cfg_find_float_array(rbin2_local_cfg_t *cfg, char *d, int bytes)
{
    rbin2_array_t *a = 0, *b = 0;
    rbin2_cfg_item_t *i;
    rbin2_string_t **v;
    int k;
    float j;

    if (!cfg)
    {
        goto end;
    }

    i = (rbin2_cfg_item_t *)rbin2_cfg_queue_find(cfg->cfg, d, bytes);

    if (i && rbin2_cfg_item_is_array(i))
    {
        b = i->value.array;
        goto end;
    }

    b = rbin2_local_cfg_find_array(cfg->parent, d, bytes);

    if (!b)
    {
        goto end;
    }

end:

    if (b)
    {
        v = (rbin2_string_t **)b->slot;
        a = rbin2_array_new_h(cfg->heap, b->nslot, sizeof(float));

        for (k = 0; k < b->nslot; ++k)
        {
            j = atof(v[k]->data);
            *((float *)rbin2_array_push(a)) = j;
        }
    }

    return a;
}

rbin2_array_t *rbin2_local_cfg_find_array(rbin2_local_cfg_t *cfg, char *d, int bytes)
{
    rbin2_array_t *a = 0;
    rbin2_cfg_item_t *i;

    if (!cfg)
    {
        goto end;
    }

    i = (rbin2_cfg_item_t *)rbin2_cfg_queue_find(cfg->cfg, d, bytes);

    if (i && rbin2_cfg_item_is_array(i))
    {
        a = i->value.array;
        goto end;
    }

    a = rbin2_local_cfg_find_array(cfg->parent, d, bytes);
end:
    return a;
}


rbin2_heap_block_t *rbin2_heap_block_new(int size)
{
    rbin2_heap_block_t *b;
    char *p;
    //p=(char*)rbin2_memalign(RBIN2_HEAP_ALIGNMENT,size);
    p = rbin2_malloc(rbin2_round(sizeof(*b) + size, 8));
    b = rbin2_align_ptr(p + size, 8);
    /*
    if((char*)b+sizeof(rbin2_heap_block_t)>p+rbin2_round(sizeof(*b)+size,8))
    {
        rbin2_debug("found bug\n");
        exit(0);
    }*/
    b->next = 0;
    b->last = b->first = (uint8_t *)p;
    b->end = (uint8_t *)b->first + size;
    b->failed = 0;
    //rbin2_debug("size=%d,first=%p,%d\n",size,b->first,(int)(b->first)%16);
    return b;
}

rbin2_heap_t *rbin2_heap_new2(size_t size, int align_size)
{
    rbin2_heap_t *p;
    p = (rbin2_heap_t *)rbin2_calloc(1, sizeof(rbin2_heap_t));
    p->max = (size < RBIN2_MAX_ALLOC_FROM_HEAP) ? size : RBIN2_MAX_ALLOC_FROM_HEAP;
    p->size = size;
    p->large = 0;
    p->first = p->current = rbin2_heap_block_new(size);
    p->align = align_size;
    return p;
}

rbin2_heap_t *rbin2_heap_new(size_t size)
{
    return rbin2_heap_new2(size, RBIN2_ALIGNMENT);
}

rbin2_strbuf_t *rbin2_strbuf_new(int init_len, float rate)
{
    rbin2_strbuf_t *b;
    char *data;
    data = (char *)rbin2_malloc(init_len);

    if (!data)
    {
        b = 0;
        goto end;
    }

    b = (rbin2_strbuf_t *)rbin2_malloc(sizeof(*b));
    b->data = data;
    b->length = init_len;
    b->pos = 0;
    b->rate = 1.0f + rate;
end:
    return b;
}

rbin2_st_t *rbin2_new()
{
    rbin2_st_t *rb;
    rb = (rbin2_st_t *)rbin2_malloc(sizeof(rbin2_st_t));
    rb->heap = rbin2_heap_new(4096);
    rb->f = NULL;
    rb->fn = NULL;
    rb->buf = rbin2_strbuf_new(4096, 1);
    rbin2_queue_init(&(rb->list));
    return rb;
}

void *rbin2_heap_malloc_large(rbin2_heap_t *heap, size_t size)
{
    rbin2_heap_large_t *l;
    void *p;
    //rbin2_debug("malloc larg=%d\n",size);
    p = rbin2_malloc(size);

    if (!p)
    {
        return 0;
    }

    l = (rbin2_heap_large_t *)rbin2_heap_malloc(heap, sizeof(rbin2_heap_large_t));
    l->data = p;
    l->size = size;
    l->next = heap->large;
    heap->large = l;
    return p;
}

void *rbin2_heap_malloc_block(rbin2_heap_t *heap, size_t size)
{
    uint8_t *m;
    rbin2_heap_block_t *newb;
    newb = rbin2_heap_block_new(heap->size);
    m = rbin2_align_ptr(newb->last, heap->align);

    if (m + size > newb->end)
    {
        m = rbin2_heap_malloc_large(heap, size);
    }
    else
    {
        newb->last = m + size;
    }

    heap->current->next = newb;
    heap->current = newb;
    return m;
}

#ifdef USE_HEAP

void *rbin2_heap_malloc(rbin2_heap_t *heap, size_t size)
{
    return malloc(size);
}
#else

void *rbin2_heap_malloc(rbin2_heap_t *heap, size_t size)
{
    rbin2_heap_block_t *b;
    uint8_t *m;
    int align;

    if (size > heap->max)
    {
        return rbin2_heap_malloc_block(heap, size);
    }

    align = heap->align;
    b = heap->current;

    if (align > 1)
    {
        m = rbin2_align_ptr(b->last, align);
    }
    else
    {
        m = b->last;
    }

    if ((int)((b->end - m)) >= size)
    {
        b->last = m + size;
        return m;
    }

    return rbin2_heap_malloc_block(heap, size);
}
#endif


rbin2_item_t *rbin2_new_item(rbin2_st_t *rb)
{
    rbin2_item_t *item;
    item = (rbin2_item_t *)rbin2_heap_malloc(rb->heap, sizeof(rbin2_item_t));
    item->fn = NULL;
    item->data = NULL;
    item->pos = -1;
    item->len = item->seek_pos = 0;
    item->buf_pos = 0;
    item->reverse = 0;
    item->rb = rb;
    return item;
}

rbin2_string_t *rbin2_heap_dup_string(rbin2_heap_t *h, char *s, int sl)
{
    rbin2_string_t *str;
    str = (rbin2_string_t *)rbin2_heap_malloc(h, sizeof(*str) + sl);
    str->len = sl;
    str->data = (char *)str + sizeof(*str);

    if (s)
    {
        memcpy(str->data, s, sl);
    }

    return str;
}

void rbin2_rbin_reverse_data(unsigned char *p, int len)
{
    unsigned char *e;
    e = p + len;

    while (p < e)
    {
        *p = ~*p;
        ++p;
    }
}

int rbin2_queue_push(rbin2_queue_t *q, rbin2_queue_node_t *n)
{
    n->prev = q->push;

    if (q->push)
    {
        q->push->next = n;
    }

    n->next = 0;
    q->push = n;

    if (!q->pop)
    {
        q->pop = n;
    }

    if (q->listener)
    {
        q->listener(q->data);
    }

    ++q->length;
    return 0;
}

char *rbin2_heap_dup_str2(rbin2_heap_t *heap, char *data, int len)
{
    char *d = 0;

    if (len <= 0)
    {
        goto end;
    }

    d = (char *)rbin2_heap_malloc(heap, len + 1);
    memcpy(d, data, len);
    d[len] = 0;
end:
    return d;
}

char *rbin2_heap_dup_str(rbin2_heap_t *heap, char *s)
{
    return rbin2_heap_dup_str2(heap, s, s ? AISP_TSL_strlen(s) : 0);
}

int rbin2_read(rbin2_st_t *rb, char *fn)
{
    FILE *f;
    int i;
    int ret;
    int n, vi;
    rbin2_heap_t *heap;
    rbin2_item_t *item;
    heap = rb->heap;
    f = fopen(fn, "rb");

    if (!f)
    {
        ret = -1;
        goto end;
    }

    // number of item
    ret = fread((char *) & (n), 4, 1, f);

    if (ret != 1)
    {
        ret = -1;
        goto end;
    }

    for (i = 0; i < n; ++i)
    {
        item = rbin2_new_item(rb);
        // length of item name
        ret = fread((char *) & (vi), 4, 1, f);

        if (ret != 1)
        {
            ret = -1;
            goto end;
        }

        // item name
        item->fn = rbin2_heap_dup_string(heap, 0, vi);
        ret = fread(item->fn->data, item->fn->len, 1, f);

        if (ret != 1)
        {
            ret = -1;
            goto end;
        }

        rbin2_rbin_reverse_data((unsigned char *)item->fn->data, item->fn->len);

        if (item->fn->data[item->fn->len - 1] == 'r')
        {
            item->reverse = 1;
        }
        else
        {
            item->reverse = 0;
        }

        // item data offset
        ret = fread((char *) & (vi), 4, 1, f);

        if (ret != 1)
        {
            ret = -1;
            goto end;
        }

        item->pos = vi;
        // item data length
        ret = fread((char *) & (vi), 4, 1, f);

        if (ret != 1)
        {
            ret = -1;
            goto end;
        }

        item->len = vi;
        item->data = NULL;
        rbin2_queue_push(&(rb->list), &(item->q_n));
    }

    rb->fn = rbin2_heap_dup_str(rb->heap, fn);
    ret = 0;
end:

    if (ret != 0)
    {
        if (f)
        {
            fclose(f);
        }
    }
    else
    {
        rb->f = f;
    }

    return ret;
}

void *rbin2_heap_zalloc(rbin2_heap_t *heap, size_t size)
{
    void *p;
    p = rbin2_heap_malloc(heap, size);

    if (p)
    {
        memset(p, 0, size);
    }

    return p;
}

rbin2_cfg_queue_t *rbin2_cfg_queue_new_h(rbin2_heap_t *heap)
{
    rbin2_cfg_queue_t *c;
    c = (rbin2_cfg_queue_t *)rbin2_heap_malloc(heap, sizeof(*c));
    rbin2_queue_init(&(c->queue));
    c->heap = heap;
    return c;
}

rbin2_local_cfg_t *rbin2_local_cfg_new_h(rbin2_heap_t *h)
{
    rbin2_local_cfg_t *c;
    c = (rbin2_local_cfg_t *)rbin2_heap_zalloc(h, sizeof(*c));
    c->cfg = rbin2_cfg_queue_new_h(h);
    c->heap = h;
    return c;
}

int rbin2_cfg_file_init(rbin2_cfg_file_t *f)
{
    f->main = rbin2_local_cfg_new_h(f->heap);
    f->cur = f->main;
    rbin2_string_set_s(&(f->main->name), "main");
    rbin2_queue_push(&(f->cfg_queue), &(f->main->q_n));
    f->scope = 0;
    f->included = 0;
    f->escaped = 0;
    return 0;
}

rbin2_cfg_file_t *rbin2_cfg_file_new_ex(int buf_size, int heap_size)
{
    rbin2_cfg_file_t *f;
    f = (rbin2_cfg_file_t *)rbin2_malloc(sizeof(*f));
    rbin2_queue_init(&(f->cfg_queue));
    f->heap = rbin2_heap_new(heap_size);
    f->tok = rbin2_strbuf_new(buf_size, 1);
    f->value = rbin2_strbuf_new(buf_size, 1);
    f->var = rbin2_strbuf_new(buf_size, 1);
    //f->comment=rbin2_strbuf_new(buf_size,1);
    f->quoted = 0;
    f->included = 0;
    rbin2_cfg_file_init(f);
    return f;
}

rbin2_cfg_file_t *rbin2_cfg_file_new()
{
    return rbin2_cfg_file_new_ex(2048, 4096);
}

rbin2_item_t *rbin2_get(rbin2_st_t *rb, char *name, int len)
{
    rbin2_queue_node_t *qn;
    rbin2_item_t *item;

    for (qn = rb->list.pop; qn; qn = qn->next)
    {
        item = data_offset(qn, rbin2_item_t, q_n);

        if (rbin2_string_cmp(item->fn, name, len) == 0)
        {
            return item;
        }
    }

    return NULL;
}

rbin2_string_t *rbin2_string_new(int len)
{
    rbin2_string_t *s;
    s = (rbin2_string_t *)rbin2_malloc(len + sizeof(*s));
    s->len = len;

    if (len > 0)
    {
        s->data = (char *)s + sizeof(*s);
    }
    else
    {
        s->data = 0;
    }

    return s;
}

int rbin2_load_item(rbin2_st_t *rb, rbin2_item_t *item, int use_heap)
{
    FILE *f = rb->f;
    int ret;
    // rbin2_debug("[%.*s] pos=%d r=%d\n",item->fn->len,item->fn->data,item->pos,item->reverse);
    ret = fseek(f, item->pos, SEEK_SET);

    if (ret != 0)
    {
        goto end;
    }

    if (use_heap)
    {
        item->data = rbin2_heap_dup_string(rb->heap, 0, item->len);
    }
    else
    {
        item->data = rbin2_string_new(item->len);
    }

    ret = fread(item->data->data, item->len, 1, f);

    if (ret != 1)
    {
        ret = -1;
        goto end;
    }

    if (item->reverse)
    {
        rbin2_rbin_reverse_data((unsigned char *)item->data->data, item->data->len);
    }

    ret = 0;
end:
    return ret;
}

rbin2_item_t *rbin2_get2(rbin2_st_t *rb, const char *name, int len)
{
    rbin2_item_t *item;
    int ret;
    item = rbin2_get(rb, (char *)name, len);

    if (item && !item->data)
    {
        ret = rbin2_load_item(rb, item, 1);

        if (ret != 0)
        {
            return NULL;
        }
    }

    return item;
}




rbin2_string_t *rbin2_cfg_queue_dup_string(rbin2_cfg_queue_t *cfg, char *v, int vbytes)
{
    rbin2_string_t *sv;

    if (vbytes <= 0)
    {
        v = 0;
    }

    sv = (rbin2_string_t *)rbin2_heap_malloc(cfg->heap, sizeof(*sv));

    if (vbytes <= 0)
    {
        sv->len = 0;
        sv->data = 0;
    }
    else
    {
        sv->len = vbytes;
        sv->data = (char *)rbin2_heap_malloc(cfg->heap, vbytes + 1);
        memcpy(sv->data, v, vbytes);
        sv->data[vbytes] = 0;
    }

    return sv;
}

int rbin2_queue_remove(rbin2_queue_t *q, rbin2_queue_node_t *n)
{
    if (q->length <= 0)
    {
        return 0;
    }

    if (n->prev)
    {
        n->prev->next = n->next;
    }
    else
    {
        q->pop = n->next;
    }

    if (n->next)
    {
        n->next->prev = n->prev;
    }
    else
    {
        q->push = n->prev;
    }

    n->prev = n->next = 0;
    --q->length;
    return 0;
}

void rbin2_cfg_queue_remove(rbin2_cfg_queue_t *c, rbin2_cfg_item_t *item)
{
    rbin2_queue_remove(&(c->queue), &(item->n));
}

int rbin2_cfg_queue_add(rbin2_cfg_queue_t *c, rbin2_cfg_item_t *item)
{
    return rbin2_queue_push(&(c->queue), &(item->n));
}


int rbin2_cfg_queue_add_string(rbin2_cfg_queue_t *cfg, char *k, int kbytes, char *v, int vbytes)
{
    rbin2_heap_t *h = cfg->heap;
    rbin2_cfg_item_t *item;
    rbin2_string_t *sv;
    sv = rbin2_cfg_queue_dup_string(cfg, v, vbytes);
    item = (rbin2_cfg_item_t *)rbin2_cfg_queue_find(cfg, k, kbytes);

    if (rbin2_str_equal_s(v, vbytes, "nil"))
    {
        if (item)
        {
            rbin2_cfg_queue_remove(cfg, item);
        }
    }
    else
    {
        if (!item)
        {
            item = (rbin2_cfg_item_t *)rbin2_heap_malloc(h, sizeof(*item));
            item->key = rbin2_cfg_queue_dup_string(cfg, k, kbytes);
            rbin2_cfg_queue_add(cfg, item);
        }

        item->type = RBIN2_CFG_STRING;
        item->value.str = sv;
    }

    return 0;
}

int rbin2_cfg_file_add_var(rbin2_cfg_file_t *cfg, char *k, int kbytes, char *v, int vbytes)
{
    return rbin2_cfg_queue_add_string(cfg->main->cfg, k, kbytes, v, vbytes);
}

void rbin2_strbuf_expand(rbin2_strbuf_t *s, int bytes)
{
    int left, alloc;
    char *p;
    int t1, t2;
    left = s->length - s->pos;

    if (bytes > left)
    {
        t1 = s->length * s->rate;
        t2 = s->pos + bytes;
        alloc = max(t1, t2); //s->length*s->rate,s->pos+bytes);
        p = s->data;
        s->data = (char *)rbin2_malloc(alloc);
        s->length = alloc;
        memcpy(s->data, p, s->pos);
        rbin2_free(p);
    }

    return;
}

void rbin2_strbuf_push_c(rbin2_strbuf_t *buf, char b)
{
    if (buf->length <= buf->pos)
    {
        rbin2_strbuf_expand(buf, 1);
    }

    buf->data[buf->pos++] = b;
}

int rbin2_cfg_file_feed_expr_tok_start(rbin2_cfg_file_t *cfg, char c)
{
    int ret = 0;

    if (cfg->quoted)
    {
        if (c != cfg->quoted_char)
        {
            rbin2_strbuf_push_c(cfg->tok, c);
        }
        else
        {
            cfg->quoted = 0;
            cfg->state = CF_EXPR_TOK_WAIT_EQ;
        }
    }
    else
    {
        if (is_char(c))
        {
            rbin2_strbuf_push_c(cfg->tok, c);
        }
        else
            if (c == '=')
            {
                cfg->state = CF_EXPR_VALUE_START;
                //ret=rbin2_cfg_file_feed_expr_value_start(cfg,c);
            }
            else
                if (!AISP_TSL_isspace(c))
                {
                    ret = -1;
                }
    }

    return ret;
}

int rbin2_cfg_file_feed_expr_start(rbin2_cfg_file_t *cfg, char c)
{
    int ret = 0;

    if (is_char(c) || c == '"' || c == '\'')
    {
        cfg->state = CF_EXPR_TOK_START;
        rbin2_strbuf_reset(cfg->tok);

        if (c == '"' || c == '\'')
        {
            cfg->quoted = 1;
            cfg->quoted_char = c;
        }
        else
        {
            ret = rbin2_cfg_file_feed_expr_tok_start(cfg, c);
        }
    }
    else
        if (c == '}')
        {
            if (cfg->scope <= 0)
            {
                ret = -1;
            }
            else
            {
                --cfg->scope;
                cfg->cur = cfg->cur->parent;
            }
        }
        else
            if (c == '#')
            {
                cfg->state = CFG_COMMENT;
                rbin2_strbuf_reset(cfg->tok);
            }
            else
            {
                //rbin2_debug("c=%c\n",c);
                ret = 0;
            }

    return ret;
}


int rbin2_cfg_file_feed_expr_tok_wait_eq(rbin2_cfg_file_t *cfg, char c)
{
    if (c == '=')
    {
        cfg->state = CF_EXPR_VALUE_START;
    }

    return 0;
}

int rbin2_cfg_queue_add_lc(rbin2_cfg_queue_t *cfg, char *k, int kbytes, rbin2_local_cfg_t *lc)
{
    rbin2_heap_t *h = cfg->heap;
    rbin2_cfg_item_t *item;
    item = (rbin2_cfg_item_t *)rbin2_cfg_queue_find(cfg, k, kbytes);

    if (!item)
    {
        item = (rbin2_cfg_item_t *)rbin2_heap_malloc(h, sizeof(*item));
        item->key = rbin2_cfg_queue_dup_string(cfg, k, kbytes);
        rbin2_string_set(&(lc->name), item->key->data, item->key->len);
        rbin2_cfg_queue_add(cfg, item);
    }

    item->type = RBIN2_CFG_LC;
    item->value.cfg = lc;
    return 0;
}

rbin2_array_t *rbin2_array_new_h(rbin2_heap_t *h, uint32_t n, uint32_t size)
{
    rbin2_array_t *a;
    a = (rbin2_array_t *)rbin2_heap_malloc(h, sizeof(*a));
    a->slot_alloc = n;
    a->slot_size = size;
    a->nslot = 0;
    a->heap = h;
    a->slot = rbin2_heap_malloc(h, n * size);
    return a;
}

int rbin2_cfg_queue_add_array(rbin2_cfg_queue_t *cfg, char *k, int bytes, rbin2_array_t *a)
{
    rbin2_heap_t *h = cfg->heap;
    rbin2_cfg_item_t *item;
    item = (rbin2_cfg_item_t *)rbin2_cfg_queue_find(cfg, k, bytes);

    if (!item)
    {
        item = (rbin2_cfg_item_t *)rbin2_heap_malloc(h, sizeof(*item));
        item->key = rbin2_cfg_queue_dup_string(cfg, k, bytes);
        rbin2_cfg_queue_add(cfg, item);
    }

    item->type = RBIN2_CFG_ARRAY;
    item->value.array = a;
    return 0;
}

uint64_t rbin2_file_length(FILE *f)
{
    uint64_t len;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    return len;
}

char *rbin2_file_read_buf(char *fn, int *n)
{
    FILE *file = fopen(fn, "rb");
    char *p = 0;
    int len;

    if (file)
    {
        len = rbin2_file_length(file);
        p = (char *)rbin2_malloc(len + 1);
        len = fread(p, 1, len, file);

        if (n)
        {
            *n = len;
        }

        fclose(file);
        p[len] = 0;
    }

    return p;
}

rbin2_string_t *rbin2_string_dup_data(char *data, int len)
{
    rbin2_string_t *s;
    s = rbin2_string_new(len);

    if (s && data)
    {
        memcpy(s->data, data, len);
    }

    return s;
}


rbin2_string_t *rbin2_dir_name(char *fn, char sep)
{
    int len;
    char *s, *e;
    rbin2_string_t *str;
    len = AISP_TSL_strlen(fn);
    s = fn;
    e = s + len;

    while (e > s)
    {
        if (*e == sep)
        {
            break;
        }

        --e;
        --len;
    }

    str = rbin2_string_dup_data(fn, len);
    return str;
}

int rbin2_string_delete(rbin2_string_t *s)
{
    rbin2_free(s);
    return 0;
}


int rbin2_cfg_file_process_include(rbin2_cfg_file_t *cfg)
{
    int ret = -1;
    char *data;
    rbin2_string_t *v;
    rbin2_cfg_item_t *pth, *item;
    int n;
    rbin2_cfg_queue_t *cq;
    rbin2_strbuf_push_c(cfg->value, 0);
    cfg->included = 0;
    data = rbin2_file_read_buf(cfg->value->data, &n);

    if (!data)
    {
        rbin2_debug("%s not found.\n", cfg->value->data);
        goto end;
    }

    cfg->state = CF_EXPR_START;
    cq = cfg->cur->cfg;
    pth = rbin2_cfg_queue_find_s(cq, "pwd");

    if (pth)
    {
        rbin2_cfg_queue_remove(cq, pth);
    }

    v = rbin2_dir_name(cfg->value->data, '/');

    if (!v)
    {
        goto end;
    }

    rbin2_cfg_queue_add_string(cq, "pwd", 3, v->data, v->len);
    rbin2_string_delete(v);
    ret = rbin2_cfg_file_feed_string(cfg, data, n);
    rbin2_free(data);

    if (ret != 0)
    {
        goto end;
    }

    item = rbin2_cfg_queue_find_s(cq, "pwd");

    if (item)
    {
        rbin2_cfg_queue_remove(cq, item);
    }

    if (pth)
    {
        rbin2_cfg_queue_add(cq, pth);
    }

    cfg->state = CF_EXPR_START;
end:
    return ret;
}

int rbin2_cfg_file_feed_expr_value_tok_end(rbin2_cfg_file_t *cfg, char c)
{
    int ret = 0;

    if (c == ';')
    {
        //rbin2_strbuf_push_c(cfg->value,0);
        if (cfg->included)
        {
            ret = rbin2_cfg_file_process_include(cfg);
        }
        else
        {
            rbin2_cfg_queue_add_string(cfg->cur->cfg, cfg->tok->data, cfg->tok->pos, cfg->value->data, cfg->value->pos);
            cfg->state = CF_EXPR_START;
        }
    }
    else
        if (!AISP_TSL_isspace(c))
        {
            rbin2_debug("expect \";\"\n");
            ret = -1;
        }

    return ret;
}

int rbin2_cfg_file_feed_expr_value_tok_start(rbin2_cfg_file_t *cfg, char c)
{
    int ret = 0;

    if (c == '\\')
    {
        cfg->state = CFG_ESCAPE_START;
        return 0;
    }

    if (cfg->quoted)
    {
        if (c == cfg->quoted_char)
        {
            cfg->quoted = 0;
            cfg->state = CF_EXPR_VALUE_TOK_END;
        }
        else
        {
            rbin2_strbuf_push_c(cfg->value, c);
        }

        return 0;
    }

    if (is_value(c))
    {
        rbin2_strbuf_push_c(cfg->value, c);
    }
    else
        if (c == ';')
        {
            cfg->state = CF_EXPR_VALUE_TOK_END;
            ret = rbin2_cfg_file_feed_expr_value_tok_end(cfg, c);
        }
        else
            if (c == '$')
            {
                cfg->var_cache_state = CF_EXPR_VALUE_TOK_START;
                cfg->state = CF_VAR_START;
            }
            else
            {
                rbin2_debug("expect var value %c end.\n", c);
                ret = -1;
            }

    return ret;
}

int rbin2_cfg_file_feed_expr_value_start(rbin2_cfg_file_t *cfg, char c)
{
    rbin2_heap_t *h = cfg->heap;
    rbin2_local_cfg_t *lc;
    int ret = 0;

    if (c == '{')
    {
        rbin2_cfg_item_t *item;
        item = rbin2_cfg_queue_find(cfg->cur->cfg, cfg->tok->data, cfg->tok->pos);

        if (item && item->type == RBIN2_CFG_LC)
        {
            lc = item->value.cfg;
        }
        else
        {
            lc = rbin2_local_cfg_new_h(h);
            rbin2_cfg_queue_add_lc(cfg->cur->cfg, cfg->tok->data, cfg->tok->pos, lc);
            rbin2_queue_push(&(cfg->cfg_queue), &(lc->q_n));
            lc->parent = cfg->cur;
        }

        cfg->cur = lc;
        cfg->state = CF_EXPR_START;
        ++cfg->scope;
        ret = rbin2_cfg_file_feed_expr_start(cfg, c);
    }
    else
        if (c == '[')
        {
            cfg->state = CFG_ARRAY_START;
            cfg->array = rbin2_array_new_h(cfg->heap, 5, sizeof(rbin2_string_t *));
            rbin2_cfg_queue_add_array(cfg->cur->cfg, cfg->tok->data, cfg->tok->pos, cfg->array);
        }
        else
            if (is_char(c) || c == '$' || c == '"')
            {
                cfg->state = CF_EXPR_VALUE_TOK_START;
                rbin2_strbuf_reset(cfg->value);
                cfg->quoted = c == '"';

                if (cfg->quoted)
                {
                    cfg->quoted_char = c;
                }
                else
                {
                    ret = rbin2_cfg_file_feed_expr_value_tok_start(cfg, c);
                }
            }
            else
                if (!AISP_TSL_isspace(c))
                {
                    rbin2_debug("expect expr value start %c.\n", c);
                    ret = -1;
                }

    return ret;
}

int rbin2_cfg_file_feed_escape_start(rbin2_cfg_file_t *cfg, char c)
{
    if (c == 'x' || c == 'X')
    {
        cfg->escape_char = 0;
        cfg->state = CFG_ESCAPE_X1;
    }
    else
        if (c >= '0' && c <= '7')
        {
            cfg->escape_char = c - '0';
            cfg->state = CFG_ESCAPE_O1;
        }
        else
        {
            switch (c)
            {
                case 't':
                    rbin2_strbuf_push_c(cfg->value, '\t');
                    break;

                case 'n':
                    rbin2_strbuf_push_c(cfg->value, '\n');
                    break;

                case 'r':
                    rbin2_strbuf_push_c(cfg->value, '\r');
                    break;

                case '\'':
                    rbin2_strbuf_push_c(cfg->value, '\'');
                    break;

                case '\"':
                    rbin2_strbuf_push_c(cfg->value, '\"');
                    break;

                case '\\':
                    rbin2_strbuf_push_c(cfg->value, '\\');
                    break;

                default:
                    rbin2_strbuf_push_c(cfg->value, c);
                    break;
            }

            cfg->state = CF_EXPR_VALUE_TOK_START;
        }

    return 0;
}

int rbin2_char_to_hex(char c)
{
    int v;

    if (c >= '0' && c <= '9')
    {
        v = c - '0';
    }
    else
        if (c >= 'A' && c <= 'F')
        {
            v = c - 'A' + 10;
        }
        else
            if (c >= 'a' && c <= 'f')
            {
                v = c - 'a' + 10;
            }
            else
            {
                v = -1;
            }

    return v;
}

int rbin2_cfg_file_feed_escape_x1(rbin2_cfg_file_t *cfg, char c)
{
    int ret;
    int v;
    v = rbin2_char_to_hex(c);

    if (v == -1)
    {
        ret = -1;
    }
    else
    {
        cfg->escape_char = v;
        cfg->state = CFG_ESCAPE_X2;
        ret = 0;
    }

    return ret;
}

int rbin2_cfg_file_feed_escape_x2(rbin2_cfg_file_t *cfg, char c)
{
    int ret;
    int v;
    v = rbin2_char_to_hex(c);

    if (v == -1)
    {
        ret = -1;
    }
    else
    {
        cfg->escape_char = (cfg->escape_char << 4) + v;
        rbin2_strbuf_push_c(cfg->value, cfg->escape_char);
        cfg->state = CF_EXPR_VALUE_TOK_START;
        ret = 0;
    }

    return ret;
}

int rbin2_cfg_file_feed_escape_o1(rbin2_cfg_file_t *cfg, char c)
{
    int ret;

    if (c >= '0' && c <= '7')
    {
        cfg->escape_char = (cfg->escape_char << 2) + c - '0';
        cfg->state = CFG_ESCAPE_O2;
        ret = 0;
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int rbin2_cfg_file_feed_escape_o2(rbin2_cfg_file_t *cfg, char c)
{
    int ret;

    if (c >= '0' && c <= '7')
    {
        cfg->escape_char = (cfg->escape_char << 2) + c - '0';
        cfg->state = CF_EXPR_VALUE_TOK_START;
        ret = 0;
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int rbin2_cfg_file_feed_var_start(rbin2_cfg_file_t *cfg, char c)
{
    int ret;

    if (c == '{')
    {
        cfg->state = CF_VAR_TOK;
        ret = 0;
    }
    else
    {
        rbin2_debug("expect var { start.\n");
        ret = -1;
    }

    return ret;
}

void rbin2_strbuf_push(rbin2_strbuf_t *s, const char *buf, int bytes)
{
    if (!buf || bytes < 0)
    {
        return;
    }

    if (bytes > s->length - s->pos)
    {
        rbin2_strbuf_expand(s, bytes);
    }

    memcpy(s->data + s->pos, buf, bytes);
    s->pos += bytes;
    return;
}

int rbin2_cfg_file_feed_var_tok_start(rbin2_cfg_file_t *cfg, char c)
{
    int ret = 0;
    rbin2_string_t *n;

    if (is_char(c))
    {
        rbin2_strbuf_push_c(cfg->var, c);
    }
    else
        if (c == '}')
        {
            n = rbin2_local_cfg_find_string(cfg->cur, cfg->var->data, cfg->var->pos);

            if (n)
            {
                rbin2_strbuf_push(cfg->value, n->data, n->len);
                cfg->state = cfg->var_cache_state;
                //cfg->state=CF_EXPR_VALUE_TOK_START;
            }
            else
            {
                rbin2_debug("var %*.*s not found.\n", cfg->var->pos, cfg->var->pos, cfg->var->data);
                ret = -1;
            }
        }
        else
            if (!AISP_TSL_isspace(c))
            {
                rbin2_debug("expect expr tok start.\n");
                ret = -1;
            }

    return ret;
}

int rbin2_cfg_file_feed_var_tok(rbin2_cfg_file_t *cfg, char c)
{
    int ret;

    if (!AISP_TSL_isspace(c))
    {
        cfg->state = CF_VAR_TOK_START;
        rbin2_strbuf_reset(cfg->var);
        ret = rbin2_cfg_file_feed_var_tok_start(cfg, c);
    }
    else
    {
        ret = 0;
    }

    return ret;
}

void rbin2_cfg_file_set_state(rbin2_cfg_file_t *cfg, rbin2_cfg_file_state_t state)
{
    cfg->state = state;
    cfg->quoted = 0;
    cfg->escaped = 0;
}

void *rbin2_array_push_n(rbin2_array_t *a, uint32_t n)
{
    rbin2_heap_block_t *b;
    uint32_t size, alloc;
    void *s;
    size = n * a->slot_size;

    if (a->nslot + n > a->slot_alloc)
    {
        b = a->heap->current;

        if (((uint8_t *)a->slot + a->slot_size * a->slot_alloc == b->last) && (b->last + size <= b->end))
        {
            b->last += size;
            a->slot_alloc += n;
        }
        else
        {
            alloc = 2 * max(n, a->slot_alloc);
            s = rbin2_heap_malloc(a->heap, alloc * a->slot_size);
            memcpy(s, a->slot, a->slot_size * a->nslot);
            a->slot = s;
            a->slot_alloc = alloc;
        }
    }

    s = (char *)a->slot + a->slot_size * a->nslot;
    a->nslot += n;
    return s;
}

void *rbin2_array_push(rbin2_array_t *a)
{
    return rbin2_array_push_n(a, 1);
}

int rbin2_cfg_file_feed_array_tok_end(rbin2_cfg_file_t *cfg, char c)
{
    int ret = 0;

    //rbin2_debug("[%c]\n",c);
    if (c == ',')
    {
        rbin2_cfg_file_set_state(cfg, CFG_ARRAY_START);
    }
    else
        if (c == ']')
        {
            rbin2_cfg_file_set_state(cfg, CF_EXPR_START);
        }
        else
            if (!AISP_TSL_isspace(c))
            {
                rbin2_debug("expect array tok like \",\" or \"]\",buf found[%c]\n", c);
                ret = -1;
            }

    return ret;
}

int rbin2_cfg_file_feed_array_tok_start(rbin2_cfg_file_t *cfg, char c)
{
    rbin2_string_t *s;
    int ret = 0;

    //rbin2_debug("c=[%c]\n",c);
    if (cfg->escaped)
    {
        rbin2_strbuf_push_c(cfg->value, c);
        cfg->escaped = 0;
        return 0;
    }

    if (cfg->quoted)
    {
        if (c == cfg->quoted_char)
        {
            s = rbin2_heap_dup_string(cfg->heap, cfg->value->data, cfg->value->pos + 1);
            --s->len;
            s->data[s->len] = 0;
            ((rbin2_string_t **)rbin2_array_push(cfg->array))[0] = s;
            //rbin2_debug("[%.*s]\n",s->len,s->data);
            //cfg->state=CFG_ARRAY_TOK_END;
            rbin2_cfg_file_set_state(cfg, CFG_ARRAY_TOK_END);
        }
        else
        {
            if (c == '\\')
            {
                cfg->escaped = 1;
            }
            else
            {
                rbin2_strbuf_push_c(cfg->value, c);
            }
        }
    }
    else
    {
        if (AISP_TSL_isspace(c) || c == ',' || c == ']')
        {
            if (cfg->value->pos > 0)
            {
                s = rbin2_heap_dup_string(cfg->heap, cfg->value->data, cfg->value->pos + 1);
                --s->len;
                s->data[s->len] = 0;
                ((rbin2_string_t **)rbin2_array_push(cfg->array))[0] = s;
            }

            //cfg->state=CFG_ARRAY_TOK_END;
            rbin2_cfg_file_set_state(cfg, CFG_ARRAY_TOK_END);

            if (!AISP_TSL_isspace(c))
            {
                ret = rbin2_cfg_file_feed_array_tok_end(cfg, c);
            }
        }
        else
            if (c == '$')
            {
                cfg->var_cache_state = CFG_ARRAY_TOK_START;
                //cfg->state=CF_VAR_START;
                rbin2_cfg_file_set_state(cfg, CF_VAR_START);
            }
            else
            {
                if (cfg->value->pos == 0 && (c == '\'' || c == '"'))
                {
                    cfg->quoted = 1;
                    cfg->quoted_char = c;
                }
                else
                {
                    rbin2_strbuf_push_c(cfg->value, c);
                }
            }
    }

    return ret;
}

int rbin2_cfg_file_feed_array_start(rbin2_cfg_file_t *cfg, char c)
{
    int ret;

    if (!AISP_TSL_isspace(c))
    {
        //cfg->state=CFG_ARRAY_TOK_START;
        rbin2_strbuf_reset(cfg->value);
        //cfg->quoted=0;
        rbin2_cfg_file_set_state(cfg, CFG_ARRAY_TOK_START);
        ret = rbin2_cfg_file_feed_array_tok_start(cfg, c);
    }
    else
    {
        ret = 0;
    }

    return ret;
}

int rbin2_cfg_file_feed_comment(rbin2_cfg_file_t *cfg, char c)
{
    int len = sizeof("include") - 1;
    rbin2_strbuf_t *buf;

    if (c == '\n')
    {
        //rbin2_strbuf_reset(cfg->tok);
        cfg->state = CF_EXPR_START;
    }
    else
    {
        buf = cfg->tok;

        if (buf->pos < len)
        {
            rbin2_strbuf_push_c(buf, c);

            if (buf->pos == len)
            {
                if (AISP_TSL_strncmp(buf->data, "include", buf->pos) == 0)
                {
                    cfg->state = CF_EXPR_VALUE_START;
                    cfg->included = 1;
                }
            }
        }
    }

    return 0;
}

void rbin2_print_data_f2(FILE *f, char *data, int len, int cn)
{
    unsigned char c;
    int i;
    fprintf(f, "(%d,", len);

    for (i = 0; i < len; ++i)
    {
        c = (unsigned char)data[i];

        if (isprint(c))
        {
            fprintf(f, "%c", c);
        }
        else
        {
            fprintf(f, "\\x%02x", c);
        }

        if (cn && (c == '\n'))
        {
            fprintf(f, "\n");
        }
    }

    fprintf(f, ")\n");
}

void rbin2_print_data_f(FILE *f, char *data, int len)
{
    rbin2_print_data_f2(f, data, len, 0);
}

void rbin2_print_data(char *data, int len)
{
    rbin2_print_data_f(stdout, data, len);
}

int rbin2_cfg_file_feed_string(rbin2_cfg_file_t *c, char *d, int bytes)
{
    char *s = d, *e = d + bytes;
    int ret = -1;

    while (s < e)
    {
        //rbin2_debug("[%c]:%d\n",*s,c->state);
        switch (c->state)
        {
            case CF_EXPR_START:
                ret = rbin2_cfg_file_feed_expr_start(c, *s);
                break;

            case CF_EXPR_TOK_START:
                ret = rbin2_cfg_file_feed_expr_tok_start(c, *s);
                break;

            case CF_EXPR_TOK_WAIT_EQ:
                ret = rbin2_cfg_file_feed_expr_tok_wait_eq(c, *s);
                break;

            case CF_EXPR_VALUE_START:
                ret = rbin2_cfg_file_feed_expr_value_start(c, *s);
                break;

            case CFG_ESCAPE_START:
                ret = rbin2_cfg_file_feed_escape_start(c, *s);
                break;

            case CFG_ESCAPE_X1:
                ret = rbin2_cfg_file_feed_escape_x1(c, *s);
                break;

            case CFG_ESCAPE_X2:
                ret = rbin2_cfg_file_feed_escape_x2(c, *s);
                break;

            case CFG_ESCAPE_O1:
                ret = rbin2_cfg_file_feed_escape_o1(c, *s);
                break;

            case CFG_ESCAPE_O2:
                ret = rbin2_cfg_file_feed_escape_o2(c, *s);
                break;

            case CF_EXPR_VALUE_TOK_START:
                ret = rbin2_cfg_file_feed_expr_value_tok_start(c, *s);
                break;

            case CF_EXPR_VALUE_TOK_END:
                ret = rbin2_cfg_file_feed_expr_value_tok_end(c, *s);
                break;

            case CF_VAR_START:
                ret = rbin2_cfg_file_feed_var_start(c, *s);
                break;

            case CF_VAR_TOK:
                ret = rbin2_cfg_file_feed_var_tok(c, *s);
                break;

            case CF_VAR_TOK_START:
                ret = rbin2_cfg_file_feed_var_tok_start(c, *s);
                break;

            case CFG_ARRAY_START:
                ret = rbin2_cfg_file_feed_array_start(c, *s);
                break;

            case CFG_ARRAY_TOK_START:
                ret = rbin2_cfg_file_feed_array_tok_start(c, *s);
                break;

            case CFG_ARRAY_TOK_END:
                ret = rbin2_cfg_file_feed_array_tok_end(c, *s);
                break;

            case CFG_COMMENT:
                ret = rbin2_cfg_file_feed_comment(c, *s);
                break;

            default:
                ret = -1;
                break;
        }

        if (ret != 0)
        {
            rbin2_print_data(d, s - d);
            break;
        }

        ++s;
    }

    //rbin2_cfg_file_print(c);
    return ret;
}

int rbin2_cfg_file_feed(rbin2_cfg_file_t *c, char *d, int bytes)
{
    int ret;
    //rbin2_debug("%*.*s\n",bytes,bytes,d);
    c->state = CF_EXPR_START;
    c->cur = c->main;
    ret = rbin2_cfg_file_feed_string(c, d, bytes);
    return ret;
}

int rbin2_heap_reset_large(rbin2_heap_t *heap)
{
    rbin2_heap_large_t *l;

    for (l = heap->large; l; l = l->next)
    {
        rbin2_free(l->data);
    }

    return 0;
}

int rbin2_heap_block_delete(rbin2_heap_block_t *b)
{
    rbin2_free(b->first);
    return 0;
}

int rbin2_heap_delete(rbin2_heap_t *heap)
{
    rbin2_heap_block_t *p, *n;
    rbin2_heap_reset_large(heap);

    for (p = heap->first; p; p = n)
    {
        n = p->next;
        rbin2_heap_block_delete(p);

        if (!n)
        {
            break;
        }
    }

    rbin2_free(heap);
    return 0;
}

int rbin2_strbuf_delete(rbin2_strbuf_t *b)
{
    rbin2_free(b->data);
    rbin2_free(b);
    return 0;
}

int rbin2_cfg_file_delete(rbin2_cfg_file_t *c)
{
    rbin2_heap_delete(c->heap);
    rbin2_strbuf_delete(c->tok);
    rbin2_strbuf_delete(c->value);
    rbin2_strbuf_delete(c->var);
    //rbin2_strbuf_delete(c->comment);
    rbin2_free(c);
    return 0;
}


void rbin2_delete(rbin2_st_t *rb)
{
    rbin2_strbuf_delete(rb->buf);

    if (rb->f)
    {
        fclose(rb->f);
    }

    rbin2_heap_delete(rb->heap);
    rbin2_free(rb);
}

