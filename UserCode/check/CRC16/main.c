//
// Created by 15902 on 2026/11/9.
//

#include "main.h"
#include "crc16.h"
#include <stdio.h>

uint8_t data[DATA_LEN] = {0};

uint8_t data_len = sizeof(data) / sizeof(data[0]);

int main(void)
{
    unsigned int value = 0;

    for (uint8_t i = 0; i < data_len; i++)
    {
        printf("请输入你的第 %u 个要检测的数据（0~255）",i + 1);
        if(scanf("%u",&value) == 1 && value <=255)
        {
            data[i] = value;
        }else
        {
            printf("输入无效，程序结束。\n");
            return 1;
        }
        data[i] = (uint8_t)value;
    }

    uint16_t result = crc16(data,data_len);
    printf("CRC16 = %u,十六进制 ：0x%04X",(unsigned int)result,(unsigned int)result);
    return 0;
}
