//
// Created by 15902 on 2026/11/7.
//

#ifndef EMBEDDED_ALGORITHMS_DOUBLE_BUFFER_H
#define EMBEDDED_ALGORITHMS_DOUBLE_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define DB_SIZE 16          // 每块缓冲区的长度（一帧数据的点数）

/*
 * 双缓冲区 / 乒乓缓冲（Ping-Pong Buffer）
 * ------------------------------------------------------------------
 *  buf0 / buf1 : 两块真实内存，轮流扮演 active 和 shadow，不固定哪个缓冲区位读写，只看用过扮演随时变化
 *  active      : 当前「生效」的一块，读者（DDS硬件 / 消费任务）只读这里
 *  shadow      : 当前「后台」的一块，写者（CPU / 生产任务）只写这里
 *  widx        : shadow 里已经写了多少个
 *  active_len  : active 这块的有效长度（上次 swap 那一刻的 widx）
 *  swap_cnt    : 切换次数，调试用
 *
 * 铁律：
 *   写者永远只写 shadow，写完调 DB_swap 发布；
 *   读者永远只读 active，根本不关心后台在改什么。
 *   => 读者拿到的永远是一帧完整数据，绝不会读到「写了一半」的残帧。
 * ------------------------------------------------------------------
 * 和环形缓冲的区别：双缓冲只有两份、不排队、新的直接覆盖旧的；
 * 环形缓冲是一队数据、要排队、满了才丢。
 */

typedef struct
{
    uint16_t buf0[DB_SIZE];
    uint16_t buf1[DB_SIZE];

    uint16_t *volatile active;      // 读者用这块
    uint16_t *volatile shadow;      // 写者用这块

    volatile uint16_t widx;
    volatile uint16_t active_len;
    volatile uint32_t swap_cnt;
} Doublebuffer_t;



void DB_Init(Doublebuffer_t *db);

bool DB_write(Doublebuffer_t *db, uint16_t data);       // 往 shadow 写一个，写满返回 false
bool DB_swap(Doublebuffer_t *db);                       // 发布：shadow 升级为 active，空帧不切


#endif //EMBEDDED_ALGORITHMS_DOUBLE_BUFFER_H
