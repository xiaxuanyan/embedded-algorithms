#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "Double_buffer.h"

// 观察两块真实数组及其当前角色，只展示每块数组的前 4 个位置。
static void show_state(const Doublebuffer_t *db, const char *step)
{
    printf("\n--- %s ---\n", step);
    printf("active（读）指向 %s，shadow（写）指向 %s\n",
           db->active == db->buf0 ? "buf0" : "buf1",
           db->shadow == db->buf0 ? "buf0" : "buf1");
    printf("buf0 前4个：%u %u %u %u\n",
           (unsigned)db->buf0[0], (unsigned)db->buf0[1],
           (unsigned)db->buf0[2], (unsigned)db->buf0[3]);
    printf("buf1 前4个：%u %u %u %u\n",
           (unsigned)db->buf1[0], (unsigned)db->buf1[1],
           (unsigned)db->buf1[2], (unsigned)db->buf1[3]);
    printf("active_len = %u，widx = %u，切换次数 = %lu\n",
           (unsigned)db->active_len, (unsigned)db->widx,
           (unsigned long)db->swap_cnt);

    // 读者只能把 active_len 范围内的数据当作有效数据。
    printf("读者看到：");
    if (db->active_len == 0) printf("（还没有发布数据）");
    for (uint16_t i = 0; i < db->active_len; i++)
        printf("%u ", (unsigned)db->active[i]);
    printf("\n");
}

int main(void)
{
    //将控制台的输出代码页设置为 UTF-8
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    Doublebuffer_t db;
    DB_Init(&db);
    show_state(&db, "1. 初始化");

    // 写入第一帧，但还没有发布，读者暂时看不到它。
    for (uint16_t i = 1; i <= 4; i++)
        if (!DB_write(&db, i * 10)) return 1;
    show_state(&db, "2. 写完第一帧，尚未切换");

    if (!DB_swap(&db)) return 1;
    show_state(&db, "3. 发布第一帧");

    // 先只写一半第二帧，观察读者仍然看到第一帧。
    if (!DB_write(&db, 100) || !DB_write(&db, 200)) return 1;
    show_state(&db, "4. 第二帧写了一半，尚未切换");

    if (!DB_write(&db, 300) || !DB_write(&db, 400)) return 1;
    if (!DB_swap(&db)) return 1;
    show_state(&db, "5. 写完并发布第二帧");
    return 0;
}
