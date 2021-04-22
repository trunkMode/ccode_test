#ifndef __BITMAP__H__
#define __BITMPA_H__
#include <stdint.h>

typedef struct _bitmap {
    int len;
    uint8_t data[0];
} bitmap_t;

#define BITMAP_HEAD_LEN         sizeof(bitmap_t)
#define BITMAP_BYTE(bitnum)     (bitnum / (sizeof(uint8_t) * 8))
#define BITMAP_OFFSET(bitnum)   (bitnum % (sizeof(uint8_t) * 8))

#define BITNUM_TO_LEN(bitnum)   (BITMAP_HEAD_LEN + bitnum/(sizeof(uint8_t)*8) + 1)
#define LEN_TO_BITNUM(map_len)  ((map_len - sizeof(bitmap_t)) * sizeof(uint8_t) * 8)


bitmap_t *bitmap_new(int bitnum);
void bitmap_delete(bitmap_t *map);
int bitmap_set(bitmap_t *map, int bitnum);
int bitmap_clr(bitmap_t *map, int bitnum);
int bitmap_clr_all(bitmap_t *map);
int is_bitmap_set(bitmap_t *map, int bitnum);
int get_bitmap_set_count(bitmap_t *map);

int is_bitmap_range_zero(bitmap_t *map, int start_bit, int end_bit);
int bitmap_set_range(bitmap_t *map, int start_bit, int end_bit);
int bitmap_clr_range(bitmap_t *map, int start_bit, int end_bit);
#endif /* __BITMAP_H__ */
