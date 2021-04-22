
static int is_range_overlap(int start1, int end1, int start2, int end2)
{
    if (!(end1 < start2 || start1 > end2)) {
        return 1;
    }    
    return 0;
}

static int is_range_adjacent(int start1, int end1, int start2, int end2)
{
    if (start1 > end2 && (start1 - end2) == 1) { 
        return 1;
    }

    if (start2 > end1 && (start2 - end1) == 1) { 
        return 1;
    }

    return 0;
}
int range_sort_merge(range_node_t *range, int cnt)
{
    uint32_t tmp, merge_cnt = 0;
    range_node_t *tmp_range = NULL;
    int i, j, min_index = 0;

    for (i = 0; i < cnt - 1; i++) {
        min_index = i;
        for (j = i + 1; j < cnt; j++) {
            if (range[j].start < range[min_index].start) {
                min_index = j;
            }
        }

        if (min_index != i) {
            tmp = range[min_index].start;
            range[min_index].start = range[i].start;
            range[i].start = tmp;

            tmp = range[min_index].end;
            range[min_index].end = range[i].end;
            range[i].end = tmp;
        }
    }

    /* prepare mem */
    tmp_range = malloc(sizeof(*tmp_range) * cnt);
    if (!tmp_range) {
        return -1;
    }

    memset(tmp_range, 0x00, sizeof(*tmp_range) *cnt);
    tmp_range[0] = range[0];
    for (i = 1, j = 0; i < cnt; i++) {
        if (range[i].start == 0 && range[i].end == 0)
            continue;
        if (is_range_overlap(range[i].start, range[i].end,
                    tmp_range[j].start, tmp_range[j].end) ||
                is_range_adjacent(range[i].start, range[i].end,
                    tmp_range[j].start, tmp_range[j].end)) { 
            tmp_range[j].start = MIN(tmp_range[j].start, range[i].start);
            tmp_range[j].end = MAX(tmp_range[j].end, range[i].end);
        } else {
            tmp_range[++j] = range[i];
        }
    }
    memcpy(range, tmp_range, sizeof(*range) *cnt);

    for (i = 0; i < cnt; i++) {
        if (range[i].start && range[i].end)
            merge_cnt++;
    }

    /* free mem */
    free(tmp_range);
    return merge_cnt;
}
