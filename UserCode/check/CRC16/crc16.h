//
// Created by 15902 on 2026/11/9.
//

#ifndef EMBEDDED_ALGORITHMS_CRC16_H
#define EMBEDDED_ALGORITHMS_CRC16_H
#include <stdint.h>

/*
 * 这里说的是CRC16‑CCITT-FALSE版本
 * CRC16 初始值是0xFFFF，多项式是0x1021，高位先行 MSB first
 * 如果数据位是8位，则先 <<8，把数据放到高字节，从CRC最高位开始参与运算
 * 分两步：
 * 第一步：CRC16寄存器先和当前要校验的数据进行异或  crc ^= (data << 8);
 * 第二步：对异或之后的 nowCRC16，逐bit处理，一共循环8次（8bit数据）      //循环几次则看验证的数据是几bit的数据
 *     (1). nowCRC 判断最高位(bit15, 0x8000)是否为1
 *          如果是1：先左移1位，再和0x1021异或
 *          如果是0：直接左移1位，不异或
 * 如果是多字节数据校验：
 * 每个字节都经历上面整套流程；后一个字节的CRC起始值 = 上一个字节运算完成后的CRC结果
 *
 *
 * CRC8也一样，只是检测8bit的数据就不用<<8了
 * 多项式是 0x07  ,CRC8初始值是0xff
 * 起始CRC的初始值范围是0~0xff  and  0~0xffff,看情况使用
 */

uint16_t crc16(uint8_t *data, uint8_t data_len);



#endif //EMBEDDED_ALGORITHMS_CRC16_H
