#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"

bitmap_t *bitmap_new(int bitnum)
{
    int len = BITNUM_TO_LEN(bitnum);
    bitmap_t *map = NULL;

    if (!(map = malloc(len))) {
        return NULL;
    }

    memset(map, 0x00, len);
    map->len = len;

    return map;
}

void bitmap_delete(bitmap_t *map)
{
    if (!map)
        return;
    free(map);
}

static int __bitmap_set_bit(bitmap_t *map, int bitnum)
{
    int byte = BITMAP_BYTE(bitnum), offset = BITMAP_OFFSET(bitnum);

    map->data[byte] |= 1 << offset;
    return 0;
}

int bitmap_set(bitmap_t *map, int bitnum)
{
    int len = BITNUM_TO_LEN(bitnum);

    if (!map || len > map->len) {
        return -1;
    }
    __bitmap_set_bit(map, bitnum);

    return 0;
}

int bitmap_set_range(bitmap_t *map, int start_bit, int end_bit)
{
    int i = start_bit;

    if (!map || start_bit > end_bit) {
        return -1;
    }

    while (i++ <= end_bit) {
        __bitmap_set_bit(map, i);
    }

    return 0;
}

static int __bitmap_clr_bit(bitmap_t *map, int bitnum)
{
    int byte = BITMAP_BYTE(bitnum), offset = BITMAP_OFFSET(bitnum);

    map->data[byte] &= ~(1 << offset);
    return 0;
}

int bitmap_clr(bitmap_t *map, int bitnum)
{
    int len = BITNUM_TO_LEN(bitnum);

    if (!map || len > map->len) {
        return -1;
    }

    __bitmap_clr_bit(map, bitnum);
    return 0;
}

int bitmap_clr_range(bitmap_t *map, int start_bit, int end_bit)
{
    int i = start_bit;

    if (!map || start_bit > end_bit) {
        return -1;
    }

    while (i++ <= end_bit) {
        __bitmap_clr_bit(map, i);
    }

    return 0;
}

static int __is_bitmap_bit_set(bitmap_t *map, int bit)
{
    int byte = BITMAP_BYTE(bit), offset = BITMAP_OFFSET(bit);

    return (map->data[byte] & (1 << offset)) ? 1 : 0;
}

int is_bitmap_set(bitmap_t *map, int bitnum)
{
    int len = BITNUM_TO_LEN(bitnum);

    if (!map || len > map->len) {
        return 0;
    }

    return __is_bitmap_bit_set(map, bitnum);
}

int is_bitmap_range_zero(bitmap_t *map, int start_bit, int end_bit)
{
    int i = start_bit;

    if (!map || start_bit > end_bit) {
        return 0;
    }

    while (i++ <= end_bit) {
        if (__is_bitmap_bit_set(map, i)) {
            return 0;
        }
    }

    return 1;
}

/*
 *@note return how many bits have been set to 1
 */
int get_bitmap_set_count(bitmap_t *map)
{
    int bitnum, cnt = 0;

    if (!map) {
        return 0;
    }
    bitnum = LEN_TO_BITNUM(map->len);

    while (bitnum--) {
        if (__is_bitmap_bit_set(map, bitnum))
            cnt++;
    }

    return cnt;
}

// int is_bitmap_equal_value(bitmap_t *map, int start_bit, int end_bit, uint32_t value)
// {
//     uint32_t tmp = 0;
//     int i = start_bit, bit = 0;
// 
//     if (start_bit > end_bit) {
//         return 0; 
//     }
// 
//     while (i++ <= end_bit) {
//         if (__is_bitmap_bit_set(map, i)) {
//             tmp |= 
//         }
//         bit++;
//     }
// }

/*
 *@func check if the start to end of bitmap is equal to value
 */
// int bitmap_test_value(bitmap_t *map, int start, int end, )

int bitmap_clr_all(bitmap_t *map)
{
    int len;
    if (!map) {
        return -1;
    }
    len = map->len;
    memset(map, 0x00, map->len);
    len = map->len;
    return 0;
}

#define BITNUM 1000000
/*** bitmap test ***/
int main()
{
    int i;
    bitmap_t *map = bitmap_new(BITNUM);
    
    for (i = 1; i < BITNUM; i++) {
        bitmap_set(map, i);
        if (!is_bitmap_set(map, i)) {
            printf("can't set bit %d \n", i);
        }
        bitmap_clr(map, i);
        if (is_bitmap_set(map, i)) {
            printf("can't clr bit %d\n", i);
        }
    }
    return 0;
}

