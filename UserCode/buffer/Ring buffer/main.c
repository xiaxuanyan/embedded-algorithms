//
// Created by 15902 on 2026/11/7.
//

#include "main.h"
#include <stdio.h>
#include "Ring Buffer.h"

int main (void)
{
    Ringbuffer_t rb;
    uint16_t data = 0;
    RB_Init(&rb);
    for (uint16_t i = 0; i < 5000; i++)
    {
        //快进快出，放进就拿出来
        if (!RB_write(&rb, i))
        {
            printf("Buffer full\n");
            break;
        }

        if (RB_read(&rb, &data))
            printf("%u\n", data);
    }
    return 0;
}