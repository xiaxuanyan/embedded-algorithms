#include "Double_buffer.h"


void DB_Init(Doublebuffer_t *db)
{
    db->active = db->buf0;
    db->shadow = db->buf1;
    db->widx = 0;
    db->active_len = 0;
    db->swap_cnt = 0;
    for (uint16_t i = 0; i < DB_SIZE ; i++)
    {
        db->buf0[i] = 0;
        db->buf1[i] = 0;
    }
}

bool DB_write(Doublebuffer_t *db, uint16_t data)
{
    if (db->widx >= DB_SIZE) return false;
    db->shadow[db->widx] = data;
    db->widx += 1;
    return true;
}

bool DB_swap(Doublebuffer_t *db)
{
    if (db->widx == 0) return false;
    uint16_t *tmp = db->shadow;
    db->shadow = db->active;
    db->active = tmp;
    db->active_len = db->widx;
    db->swap_cnt += 1;
    db->widx = 0;
    return true;
}