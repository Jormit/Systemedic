#include <stdint.h>

struct circular_buf {
    int size;
    int store_i;
    uint32_t *data;
};

struct circular_buf *circular_buf_init(int size);
void circular_buf_push(struct circular_buf *buf, uint32_t val);
uint32_t circular_buf_return_nth(struct circular_buf *buf, int n);