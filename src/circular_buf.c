#include "circular_buf.h"
#include <malloc.h>

struct circular_buf *circular_buf_init(int size)
{
    struct circular_buf *new_buf = malloc(sizeof(struct circular_buf));
    new_buf->store_i = 0;
    new_buf->size = size + 1;
    new_buf->data = malloc(sizeof(uint32_t) * new_buf->size);
    for (int i = 0; i < new_buf->size; i++) 
    {
        new_buf->data[i] = 0;
    }
    return new_buf;
}

void circular_buf_push(struct circular_buf *buf, uint32_t val)
{
    buf->data[(buf->store_i)++] = val;
    (buf->store_i) %= buf->size;
}

uint32_t circular_buf_return_nth(struct circular_buf *buf, int n) 
{
    return buf->data[(buf->store_i + buf->size - n - 1) % buf->size];   
}