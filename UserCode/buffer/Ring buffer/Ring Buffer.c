#include "Ring Buffer.h"

void RB_Init(Ringbuffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

bool RB_write(Ringbuffer_t *rb, uint16_t data)
{
    if (((rb->head + 1) & RB_MASK) == rb->tail) return false;     // 满了就不写了，会多了一个字节进行判定
    rb->data[rb->head] = data;
    rb->head = (rb->head + 1) & RB_MASK;
    return true;
}

bool RB_read(Ringbuffer_t *rb, uint16_t *data)
{
    if (rb->head == rb->tail) return false;                         //空则不读
    *data = rb->data[rb->tail];
    rb->tail = (rb->tail + 1) & RB_MASK;
    return true;
}
