#include "count_min_sketch.h"
#include <stdlib.h>
#include <stdint.h>

// 4 different hash seeds for 4 independent hash functions
static const uint32_t SEEDS[CMS_ROWS] = {
    0x9747b28c,
    0x85ebca6b,
    0xc2b2ae35,
    0x27d4eb2f
};

// MurmurHash3 finalizer (fast, high quality)
static uint32_t murmur_hash(uint32_t key, uint32_t seed) {
    uint32_t h = seed ^ key;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h % CMS_COLS;
}

CountMinSketch* cms_create() {
    CountMinSketch *cms = calloc(1, sizeof(CountMinSketch));
    return cms;
}

// When a flow arrives, increment all 4 rows
void cms_update(CountMinSketch *cms, uint32_t flow_id, uint32_t count) {
    for (int r = 0; r < CMS_ROWS; r++) {
        uint32_t col = murmur_hash(flow_id, SEEDS[r]);
        cms->table[r][col] += count;
    }
}

// Query = minimum across all 4 rows (reduces collision error)
uint32_t cms_query(CountMinSketch *cms, uint32_t flow_id) {
    uint32_t min_val = UINT32_MAX;
    for (int r = 0; r < CMS_ROWS; r++) {
        uint32_t col = murmur_hash(flow_id, SEEDS[r]);
        if (cms->table[r][col] < min_val)
            min_val = cms->table[r][col];
    }
    return min_val;
}

void cms_destroy(CountMinSketch *cms) {
    free(cms);
}