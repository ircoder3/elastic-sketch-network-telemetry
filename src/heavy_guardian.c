#include "heavy_guardian.h"
#include <stdlib.h>
#include <string.h>

static uint32_t hash_flow(uint32_t flow_id, int num_buckets) {
    flow_id = ((flow_id >> 16) ^ flow_id) * 0x45d9f3b;
    flow_id = ((flow_id >> 16) ^ flow_id) * 0x45d9f3b;
    flow_id = (flow_id >> 16) ^ flow_id;
    return flow_id % num_buckets;
}

HeavyGuardian* hg_create(int num_buckets) {
    HeavyGuardian *hg = malloc(sizeof(HeavyGuardian));
    hg->buckets       = calloc(num_buckets, sizeof(HeavyBucket));
    hg->num_buckets   = num_buckets;
    return hg;
}

int hg_insert(HeavyGuardian *hg, uint32_t flow_id,
              uint32_t *evicted_id, uint32_t *evicted_count) {
    uint32_t idx  = hash_flow(flow_id, hg->num_buckets);
    HeavyBucket *b = &hg->buckets[idx];

    // CASE 1: Empty bucket
    if (b->count == 0) {
        b->flow_id = flow_id;
        b->count   = 1;
        b->reward  = 1;
        return 0;
    }

    // CASE 2: Same flow — reward it
    if (b->flow_id == flow_id) {
        b->count++;
        b->reward++;
        return 0;
    }

    // CASE 3: Vote and Demote
    if (b->reward > 0) {
        b->reward--;
    }

    if (b->reward == 0) {
        // Evict with full count so CMS gets accurate info
        *evicted_id    = b->flow_id;
        *evicted_count = b->count;
        b->flow_id     = flow_id;
        b->count       = 1;
        b->reward      = 1;
        return 1;
    }

    return 0;
}

uint32_t hg_query(HeavyGuardian *hg, uint32_t flow_id) {
    uint32_t idx = hash_flow(flow_id, hg->num_buckets);
    if (hg->buckets[idx].flow_id == flow_id)
        return hg->buckets[idx].count;
    return 0;
}

void hg_destroy(HeavyGuardian *hg) {
    free(hg->buckets);
    free(hg);
}