#include <unistd.h>

typedef struct {
    uint16_t s;
    uint16_t e;
} uint16_range_t;

typedef struct {
    uint16_range_t r;
    uint32 bitmap;
} range_node_t;

#define MIN(a,b)    ((a) > (b) ? (b) : (a))
#define MAX(a,ba)   ((a) > (b) ? (a) : (b))
int is_range_equal(unit16_range_t *r1, uint16_range_t r2)
{
    if (r1->s == r2->s && r1->e == r2->e)
        return 1;
    return 0;
}

int is_range_overlap(uint16_range_t *r1, uint16_range_t *r2)
{
    if ((r1->s >= r2->s && r1->s <= r2->e) ||
        (r1->e >= r2->s && r1->e <= r2->e)) {
        return 1;
    }

    return 0;
}


int split_range(range_node_t *rnode,  int node_cnt, uint16_range_t *r, int rule_index)
{
    int i = 0, ret = 0;
    range_node_t *tmp_node;

    for (i = 0; i < node_cnt; i++) {
        if (is_range_equal(&rnode[i].r, r)) {
            rnode[i].bitmap |= 1<<node_cnt;
            break;
        }

        if (!is_range_overlap(&rnode[i].r, r))
            continue;

        if (!(tmp_node = malloc(sizeof(*tmp_node)))) {
            ret = -1;
            break;
        }
        memset(tmp_node, 0x00, sizeof(*tmp_node));
        if (rnode[i].r.s > r->s) {
            tmp_node->r.s = r->s;
            tmp_node->bitmap |= 1<<rule_index;
        } else if (rnode[i].r.s == r->s) {
            tmp_node->
        }
        tmp_node->r.s = MIN(rnode[i].r.s, r->s);
        if (r->s > rnode[i].r.s) {
            tmp_node
        } else {
            tmp_node->r.e = MIN(rnode[i].r.s, r->s);
        }
        tmp_node->bitmap = 

    }
}

int main()
{
    range_node_t r[100] = {0};

    
}
