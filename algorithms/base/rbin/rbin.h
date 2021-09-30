/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : rbin.h
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
#ifndef __RBIN_H__
#define __RBIN_H__
#include <stdint.h>
#include <stdio.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS  /* empty */
#define __END_DECLS    /* empty */
#endif

__BEGIN_DECLS
#ifdef USE_CM4_OPTIMIZE
#define rbin2_debug(...)
#else
#define rbin2_debug(...)                                                        \
    printf("%s:%s:%d:", strrchr(__FILE__, '/') + 1, __FUNCTION__, __LINE__);    \
    printf(__VA_ARGS__);                                                        \
    fflush(stdout);
#endif

#define RBIN2_ALIGNMENT ((int)(sizeof(unsigned long)))
#define max(a, b) (((a) > (b)) ? (a) : (b))

typedef struct rbin2_st rbin2_st_t;
typedef struct rbin2_cfg_file rbin2_cfg_file_t;
typedef struct rbin2_string rbin2_string_t;
typedef struct rbin2_local_cfg rbin2_local_cfg_t;
typedef struct rbin2_queue_node rbin2_queue_node_t;
typedef struct rbin2_queue rbin2_queue_t;
typedef struct rbin2_heap_block rbin2_heap_block_t;
typedef struct rbin2_heap rbin2_heap_t;
typedef struct rbin2_heap_large rbin2_heap_large_t;
typedef struct rbin2_cfg_queue rbin2_cfg_queue_t;
typedef struct rbin2_strbuf rbin2_strbuf_t;
typedef struct rbin2_array rbin2_array_t;

typedef void (*queue_push_listener)(void *data);

struct rbin2_string
{
    char *data;
    int len;
};

struct rbin2_queue
{
    rbin2_queue_node_t *pop;
    rbin2_queue_node_t *push;
    queue_push_listener listener;
    void *data;
    int length;
};

struct rbin2_st
{
    rbin2_heap_t *heap;
    rbin2_strbuf_t *buf;

    FILE *f;  ///< file pointer
    char *fn; ///< file name

    rbin2_queue_t list; ///< file item list in this binary file
};

struct rbin2_heap_block
{
    uint8_t     *first;
    uint8_t     *last;
    uint8_t     *end;
    rbin2_heap_block_t *next;
    unsigned int failed;
};

struct rbin2_heap_large
{
    struct rbin2_heap_large *next;
    void *data;
    int size;
};

struct rbin2_heap
{
    rbin2_heap_block_t *first;
    size_t  max;
    size_t  size;
    size_t align;
    rbin2_heap_block_t *current;
    rbin2_heap_large_t *large;
};

struct rbin2_queue_node
{
    rbin2_queue_node_t *next;
    rbin2_queue_node_t *prev;
};

struct rbin2_cfg_queue
{
    rbin2_queue_t queue;      //rbin2_cfg_item_t
    rbin2_heap_t *heap;
};


struct rbin2_local_cfg
{
    rbin2_queue_node_t q_n;
    rbin2_string_t name;
    rbin2_cfg_queue_t *cfg;
    rbin2_heap_t *heap;
    rbin2_local_cfg_t *parent;
};

typedef enum
{
    CF_EXPR_START,
    CF_EXPR_TOK_START,
    CF_EXPR_TOK_WAIT_EQ,
    CF_EXPR_VALUE_START,
    CF_EXPR_VALUE_TOK_START,
    CF_EXPR_VALUE_TOK_END,
    CF_VAR_START,
    CF_VAR_TOK,
    CF_VAR_TOK_START,
    CFG_ARRAY_START,
    CFG_ARRAY_TOK_START,
    CFG_ARRAY_TOK_END,
    CFG_COMMENT,
    CFG_ESCAPE_START,
    CFG_ESCAPE_X1,
    CFG_ESCAPE_X2,
    CFG_ESCAPE_O1,
    CFG_ESCAPE_O2,
} rbin2_cfg_file_state_t;

struct rbin2_strbuf
{
    char *data;                 //raw data;
    int pos;                    //valid data size;
    int length;                 //memory size of raw data;
    float rate;                 //memory increase rate;
};

struct rbin2_array
{
    void *slot;
    uint32_t nslot;
    uint32_t slot_size;
    uint32_t slot_alloc;
    rbin2_heap_t *heap;
};

typedef enum
{
    RBIN2_CFG_STRING = 0,
    RBIN2_CFG_LC,
    RBIN2_CFG_ARRAY,
} rbin2_cfg_type_t;


typedef struct
{
    rbin2_queue_node_t n;
    rbin2_cfg_type_t type;
    rbin2_string_t *key;
    union
    {
        rbin2_string_t *str;
        struct rbin2_local_cfg *cfg;
        rbin2_array_t *array; //rbin2_string_t* array.
    } value;
} rbin2_cfg_item_t;

struct rbin2_cfg_file
{
    rbin2_st_t *rbin;
    rbin2_queue_t cfg_queue;
    rbin2_heap_t *heap;
    rbin2_local_cfg_t *main; // main configure;
    rbin2_local_cfg_t *cur;
    rbin2_cfg_file_state_t state;
    rbin2_cfg_file_state_t var_cache_state;
    rbin2_strbuf_t *tok;
    rbin2_strbuf_t *value;
    rbin2_strbuf_t *var;
    // rbin2_strbuf_t *comment; //comment;
    rbin2_array_t *array;
    int scope;
    char quoted_char;
    unsigned char escape_char;
    unsigned escaped : 1;
    unsigned quoted : 1;
    unsigned included : 1;
    unsigned use_bin : 1;
};


typedef struct
{
    rbin2_queue_node_t q_n;       ///< used to embed this item to a queue
    rbin2_st_t *rb;            ///< rbin2 this item belongs to
    rbin2_string_t *fn;        ///< name of file that this item attach to
    rbin2_string_t *data;

    int pos;                ///< item offset in rbin2 file
    int len;                ///< data length

    unsigned char *s;           ///< start of data
    unsigned char *e;           ///< end of data

    int seek_pos;            ///< current read position in item, for item_x
    int buf_pos;            ///< current read position in buffer, for item_x

    unsigned reverse: 1;        ///< revert bit or not
} rbin2_item_t;

#define STR1(x) #x
#define STR(X) STR1(X)
#define rbin2_local_cfg_find_string_s(cfg,s) rbin2_local_cfg_find_string(cfg,s,sizeof(s)-1)
#define rbin2_local_cfg_find_lc_s(cfg,s) rbin2_local_cfg_find_lc(cfg,s,sizeof(s)-1)
#define rbin2_local_cfg_update_cfg_i(lc,cfg,item,v) {v=rbin2_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=AISP_TSL_atoi(v->data);}}
#define rbin2_local_cfg_update_cfg_f(lc,cfg,item,v) {v=rbin2_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=atof(v->data);}}
#define rbin2_local_cfg_update_cfg_b(lc,cfg,item,v) {v=rbin2_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->item=(AISP_TSL_atoi(v->data)==1)?1:0;}}

#define rbin2_local_cfg_name_update_cfg_i(lc,cfg,name,item,v) {v=rbin2_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->name=AISP_TSL_atoi(v->data);}}
#define rbin2_local_cfg_name_update_cfg_f(lc,cfg,name,item,v) {v=rbin2_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->name=atof(v->data);}}
#define rbin2_local_cfg_name_update_cfg_b(lc,cfg,name,item,v) {v=rbin2_local_cfg_find_string_s(lc,STR(item)); if(v){cfg->name=(AISP_TSL_atoi(v->data)==1)?1:0;}}

#define rbin2_local_cfg_find_float_array_s(cfg,s) rbin2_local_cfg_find_float_array(cfg,s,sizeof(s)-1)
#define rbin2_local_cfg_find_int_array_s(cfg,s) rbin2_local_cfg_find_int_array(cfg,s,sizeof(s)-1)
#define rbin2_cfg_item_is_array(cfg) ((cfg)->type==RBIN2_CFG_ARRAY)

#define rbin2_local_cfg_find_array_s(cfg,s) rbin2_local_cfg_find_array(cfg,s,sizeof(s)-1)

rbin2_local_cfg_t *rbin2_local_cfg_find_lc(rbin2_local_cfg_t *cfg, char *d, int bytes);
rbin2_string_t *rbin2_local_cfg_find_string(rbin2_local_cfg_t *cfg, char *d, int bytes);
rbin2_array_t *rbin2_local_cfg_find_float_array(rbin2_local_cfg_t *cfg, char *d, int bytes);
rbin2_array_t *rbin2_local_cfg_find_int_array(rbin2_local_cfg_t *cfg, char *d, int bytes);
rbin2_array_t *rbin2_local_cfg_find_array(rbin2_local_cfg_t *cfg, char *d, int bytes);
rbin2_st_t *rbin2_new(void);
int rbin2_read(rbin2_st_t *rb, char *fn);
rbin2_cfg_file_t *rbin2_cfg_file_new(void);
rbin2_item_t *rbin2_get2(rbin2_st_t *rb, const char *name, int len);

#define rbin2_cfg_file_add_var_ks(cfg, k, v, vlen)    rbin2_cfg_file_add_var(cfg, (char *)k, sizeof(k) - 1, (char *)v, vlen)

int rbin2_cfg_file_add_var(rbin2_cfg_file_t *cfg, char *k, int kbytes, char *v, int vlen);
int rbin2_cfg_file_feed(rbin2_cfg_file_t *c, char *d, int bytes);
int rbin2_cfg_file_delete(rbin2_cfg_file_t *c);
void rbin2_delete(rbin2_st_t *rb);
rbin2_array_t *rbin2_array_new_h(rbin2_heap_t *h, uint32_t n, uint32_t size);
void rbin2_rbin_reverse_data(unsigned char *p, int len);


__END_DECLS
#endif