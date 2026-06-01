#ifndef HEAVY_GUARDIAN_H
#define HEAVY_GUARDIAN_H

#include <stdint.h>

typedef struct {
    uint32_t flow_id;
    uint32_t count;
    uint32_t reward;
} HeavyBucket;

typedef struct {
    HeavyBucket *buckets;
    int num_buckets;
} HeavyGuardian;

HeavyGuardian* hg_create(int num_buckets);
int      hg_insert(HeavyGuardian *hg, uint32_t flow_id,
                   uint32_t *evicted_id, uint32_t *evicted_count);
uint32_t hg_query(HeavyGuardian *hg, uint32_t flow_id);
void     hg_destroy(HeavyGuardian *hg);

#endif