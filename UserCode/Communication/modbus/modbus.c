//
// Created by 15902 on 2026/11/22.
//
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define COIL_LEN 100
#define DISCRETE_LEN 100
#define INPUTREG_LEN 100
#define HOLDING_LEN 100
#define MODBUS_BUF_SIZE 256
#define MODBUS_SLAVE_ADDR  0x01U   // 本机从站地址

typedef struct{
    uint8_t coil[COIL_LEN];
    uint8_t discrete[DISCRETE_LEN];
    uint16_t inputreg[INPUTREG_LEN];
    uint16_t holdingreg[HOLDING_LEN];
} modbus_t;

modbus_t modbus_data;

uint8_t rx_buf[MODBUS_BUF_SIZE];
uint8_t tx_buf[MODBUS_BUF_SIZE];

void modbus_Init(void){
    memset(&modbus_data,0,sizeof(modbus_data));
    memset(rx_buf,0,sizeof(rx_buf));
    memset(tx_buf,0,sizeof(tx_buf));
}

//CRC-16/MODBUS版，反射多项式0xA001，向右移一位验证，低位开始
uint16_t crc16(const uint8_t *data,uint16_t len)
{
    uint16_t crc = 0xffff;
    for(uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for(uint8_t j = 0; j < 8; j++)
        {
            if(crc & 0x0001){
                crc = (crc >> 1) ^ 0xA001;
            }else crc >>= 1;
        }
    }
    return crc;
}

/*======================拼写大端函数==================*/
static void put_16(uint8_t *p, uint16_t v)
{
    p[1] = (uint8_t)v ; p[0] = (uint8_t)(v >> 8);
}

static void put_32(uint8_t *p, uint32_t v)
{
    p[3] = (uint8_t)v;
    for(uint8_t i = 1; i <4; i++)
    {
        p[3-i] = (uint8_t)(v >> 8*i);
    }
}

static uint16_t put_get16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8)  | p[1]);
}

static uint32_t put_get32(const uint8_t *p)
{
     uint32_t v = (uint32_t)p[3];
     for(uint8_t i = 1; i < 4; i++)
     {
         v |= (uint32_t)p[3 - i] << (8 * i);
     }
     return v;
}

/*======================读写位辅助函数============================*/
static uint8_t mb_get_bit(const uint8_t *buf, uint16_t idx)
{
	return buf[idx];
}

static uint8_t mb_get_reg(const uint8_t *buf, uint16_t idx)
{
    return buf[idx];
}


static  void mb_set_bit(uint8_t *buf, uint16_t idx, uint8_t value)
{

	if(value){
		buf[idx / 8] |= (uint8_t)(1U << (idx % 8));
	}else {
		buf[idx / 8] &= (uint8_t)~(1U << (idx % 8));
	}
}

// 从机接收数据后进行线圈拆分到coil数组
static uint8_t mb_get_value(const uint8_t *buf, uint16_t idx)
{
    uint16_t i = idx / 8;
    uint8_t j = idx % 8;
    return (uint8_t)(buf[i] >> j) & 0x01U;
}

/*=============================================bit读取函数============================================*/

uint16_t mb_read_bits(const uint8_t *rx,                    // 从机发送主机要读的数据
                      uint8_t rx_len,                       // rx实际接收到的字节
                      uint8_t *tx,                          // 准备好数据放tx数组发送
                      uint8_t *arr,                         // 哪个数组里面取数据
                      uint16_t max_bits                     // 线圈最大长度
                      )
{
	if(rx_len != 8) return 0;

	uint16_t len = put_get16(&rx[4]);
	uint16_t start = put_get16(&rx[2]);

    if(len == 0 || len > 2000U || start > max_bits || len > (max_bits - start)) return 0;

    uint16_t crc_16 = (uint16_t)rx[6] | (uint16_t)(rx[7] << 8);
    if (crc_16 != crc16(rx,6U)) return 0;
    // bit占字节个数
    uint16_t bit_len = (uint16_t)((len + 7) / 8);
    //  除CRC，rx的长度
    uint16_t count = (uint16_t)(3U + bit_len);

    for(uint16_t i = 0; i < bit_len ; i++){
        tx[3 + i] = 0;
    }
	for(uint16_t i = 0; i < len; i++){
		uint16_t rxs = start + i;
		uint8_t fg = mb_get_bit(arr, rxs);
		mb_set_bit(&tx[3], i, fg);
	}

    tx[0] = rx[0];
    tx[1] = rx[1];
    tx[2] = (uint8_t)bit_len;
	uint16_t crc16_bit =  crc16(tx, count);
    tx[count] = (uint8_t)crc16_bit;
    tx[count + 1] = (uint8_t)(crc16_bit >> 8);

    return (uint16_t)(count + 2U);
}

/*=============================================bit发送函数============================================*/
uint16_t mb_write_bits(const uint8_t *rx, uint8_t rx_len, uint8_t *tx, uint8_t *arr, uint16_t max_bits)
{
    if (rx_len < 8) return 0;
    /*=========================================写单线圈========================================*/
    if (rx[1] == 0x05U)
    {
        uint16_t start = put_get16(&rx[2]);

        if (rx_len != 8 || start >= max_bits) return 0;

        uint16_t crc_16 = (uint16_t)rx[6] | (uint16_t)(rx[7] << 8);
        if (crc_16 != crc16(rx,6U)) return 0;

        uint16_t data = put_get16(&rx[4]);
        if (data == 0xFF00U)
        {
            arr[start] = 1;
        }else if (data == 0x0000U)
        {
            arr[start] = 0;
        }else return 0;
    }else if (rx[1] == 0x0FU){
        /*=======================================写多线圈=======================================*/
        if(rx_len < 10U) return 0;

        uint16_t bit_len = put_get16(&rx[4]);
        uint16_t start = put_get16(&rx[2]);

        if(bit_len == 0 || bit_len > 1968U  || start > max_bits || bit_len > (max_bits - start)) return 0;
        // bit占字节个数
        uint16_t byte_len = (uint16_t)((bit_len + 7) / 8);
        if (byte_len != rx[6]) return 0;
        //  除CRC，rx的长度
        uint16_t crc_start = rx[6] + 7;
        if (rx_len != (uint16_t)(crc_start + 2U)) return 0;
        uint16_t crc_16 = (uint16_t)rx[crc_start] | (uint16_t)(rx[crc_start + 1] << 8);

        if(crc_16 != crc16(rx,crc_start)) return 0;

        for(uint16_t i = 0; i < bit_len; i++)
        {
            uint16_t j = i + start;
            uint8_t value = mb_get_value(&rx[7], i);
            arr[j] = value;
        }
    }else return 0;

    memcpy(tx,rx,6U);
    uint16_t crc16_t = crc16(tx,6U);
    tx[6] = (uint8_t)crc16_t;
    tx[7] = (uint8_t)(crc16_t >> 8);

    return 8U;
}

/*=============================================寄存器读取函数============================================*/
uint16_t mb_read_reg(const uint8_t *rx,uint16_t rx_len, uint8_t *tx, uint16_t *arr, uint16_t max_reg_len)
{
    if (rx_len != 8U) return 0;

    uint16_t start = put_get16(&rx[2]);
    uint16_t len = put_get16(&rx[4]);

    if (len == 0 || len > 125U || start > max_reg_len || len > (max_reg_len - start)) return 0;

    uint16_t crc_16 = (uint16_t)rx[6] | (uint16_t)rx[7] << 8;
    if (crc_16 != crc16(rx,6U)) return 0;

    for (uint8_t i = 0; i < len; i++)
    {
        uint16_t rxs = start + i;
        put_16(&tx[3 + i*2U],arr[rxs]);
    }
    //除CRC16总长
    uint16_t count = 3 + len * 2;
    tx[0] = rx[0];
    tx[1] = rx[1];
    tx[2] = (uint8_t)(len * 2);
    uint16_t crc16_reg = crc16(tx,count);
    tx[count] = (uint8_t)crc16_reg;
    tx[count + 1] = (uint8_t)(crc16_reg >> 8);

    return count + 2U;
}

/*=============================================寄存器发送函数============================================*/
uint16_t mb_write_reg(const uint8_t *rx, uint8_t rx_len, uint8_t *tx, uint16_t *arr, uint16_t max_reg_len)
{
    if (rx_len < 8) return 0;
    /*==========================================写单个寄存器==============================================*/
    if (rx[1] == 0x06U)
    {
        uint16_t start = put_get16(&rx[2]);

        if (rx_len != 8 || start >= max_reg_len) return 0;

        uint16_t crc_16 = (uint16_t)rx[6] | (uint16_t)(rx[7] << 8);
        if (crc_16 != crc16(rx,6U)) return 0;

        arr[start] = put_get16(&rx[4]);
    }else if (rx[1] == 0x10U)
    {
        /*==========================================写单个寄存器==============================================*/
        if(rx_len < 10U) return 0;

        uint16_t start = put_get16(&rx[2]);
        uint16_t reg_len = put_get16(&rx[4]);
        if (rx[6] != reg_len * 2U) return 0;
        if (rx_len != 9U + reg_len * 2U) return 0;
        if (reg_len == 0 || reg_len > 123 || reg_len + start > max_reg_len) return 0;

        uint16_t crc_start = reg_len * 2 + 7;
        uint16_t crc_16 = (uint16_t)rx[crc_start] | (uint16_t)rx[crc_start + 1] << 8;
        if (crc_16 != crc16(rx,crc_start)) return 0;

        for (uint16_t i = 0; i < reg_len; i++)
        {
            arr[start + i] = put_get16(&rx[7 + i*2U]);
        }
    }else return 0;

    memcpy(tx,rx,6U);
    uint16_t crc16_t = crc16(tx,6U);
    tx[6] = (uint8_t)crc16_t;
    tx[7] = (uint8_t)(crc16_t >> 8);

    return 8U;
}


uint16_t modbus_process(const uint8_t *rx, uint16_t rx_len, uint8_t *tx)
{
    // 合法 Modbus 请求帧最短 8 字节；不是发给本机地址的直接忽略
    if (rx_len < 8U || rx[0] != MODBUS_SLAVE_ADDR) return 0;
    //先验证一遍CRC16
    uint16_t crc_rx = (uint16_t)rx[rx_len - 2U] | ((uint16_t)rx[rx_len - 1U] << 8);
    if (crc_rx != crc16(rx, (uint16_t)(rx_len - 2U))) return 0;

    // 按功能码分发到对应的读写函数和数据区
    switch (rx[1])
    {
    case 0x01U: // 读线圈
        return mb_read_bits(rx, (uint8_t)rx_len, tx, modbus_data.coil, COIL_LEN);

    case 0x02U: // 读离散输入（只读）
        return mb_read_bits(rx, (uint8_t)rx_len, tx, modbus_data.discrete, DISCRETE_LEN);

    case 0x03U: // 读保持寄存器
        return mb_read_reg(rx, rx_len, tx, modbus_data.holdingreg, HOLDING_LEN);

    case 0x04U: // 读输入寄存器（只读）
        return mb_read_reg(rx, rx_len, tx, modbus_data.inputreg, INPUTREG_LEN);

    case 0x05U: // 写单个线圈
    case 0x0FU: // 写多个线圈
        return mb_write_bits(rx, (uint8_t)rx_len, tx, modbus_data.coil, COIL_LEN);

    case 0x06U: // 写单个保持寄存器
    case 0x10U: // 写多个保持寄存器
        return mb_write_reg(rx, (uint8_t)rx_len, tx, modbus_data.holdingreg, HOLDING_LEN);

    default:    // 不支持的功能码
        return 0;
    }
}





























