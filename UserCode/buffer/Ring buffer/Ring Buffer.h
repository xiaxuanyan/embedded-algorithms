//
// Created by 15902 on 2026/11/7.
//

#ifndef EMBEDDED_ALGORITHMS_RING_BUFFER_H
#define EMBEDDED_ALGORITHMS_RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define RB_SIZE 16
#define RB_MASK (RB_SIZE - 1)

typedef struct
{
    uint16_t data[RB_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
}Ringbuffer_t;

void RB_Init(Ringbuffer_t *rb);
bool RB_write(Ringbuffer_t *rb, uint16_t data);
bool RB_read(Ringbuffer_t *rb, uint16_t *data);

#endif //EMBEDDED_ALGORITHMS_RING_BUFFER_H
