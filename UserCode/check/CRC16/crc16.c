//
// Created by 15902 on 2026/11/9.
//

#include "crc16.h"

uint16_t crc16(uint8_t *data,uint8_t data_len)
{
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < data_len; i++)
    {
        crc  ^= (data[i] << 8);
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }else crc = (crc << 1);
        }
    }
    return crc;
}



