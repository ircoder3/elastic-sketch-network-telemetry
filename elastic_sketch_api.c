#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

/* ── constants ─────────────────────────────────────────── */
#define MAX_FLOWS     20000
#define HG_BUCKETS    1024
#define CMS_ROWS      4
#define CMS_COLS      4096

/* ── MurmurHash3 ────────────────────────────────────────── */
static uint32_t mmh3(uint32_t k, uint32_t seed) {
    uint32_t h = seed ^ k;
    h ^= h>>16; h *= 0x85ebca6b;
    h ^= h>>13; h *= 0xc2b2ae35;
    h ^= h>>16;
    return h;
}
static uint32_t hg_hash(uint32_t id) {
    uint32_t h = id;
    h = ((h>>16)^h)*0x45d9f3b;
    h = ((h>>16)^h)*0x45d9f3b;
    h = (h>>16)^h;
    return h % HG_BUCKETS;
}

/* ── data structures ────────────────────────────────────── */
typedef struct { uint32_t id, count, reward; } Bucket;

static Bucket   hg[HG_BUCKETS];
static uint32_t cms[CMS_ROWS][CMS_COLS];
static uint32_t total_evictions  = 0;
static uint32_t active_guardians = 0;

static const uint32_t SEEDS[CMS_ROWS] = {
    0x9747b28c, 0x85ebca6b, 0xc2b2ae35, 0x27d4eb2f
};

/* ── sketch operations ──────────────────────────────────── */
static void cms_add(uint32_t id, uint32_t n) {
    for (int r=0;r<CMS_ROWS;r++)
        cms[r][mmh3(id,SEEDS[r])%CMS_COLS] += n;
}
static uint32_t cms_get(uint32_t id) {
    uint32_t mn = UINT32_MAX;
    for (int r=0;r<CMS_ROWS;r++) {
        uint32_t v = cms[r][mmh3(id,SEEDS[r])%CMS_COLS];
        if (v<mn) mn=v;
    }
    return mn==UINT32_MAX?0:mn;
}

static void insert_flow(uint32_t id) {
    uint32_t idx = hg_hash(id);
    Bucket  *b   = &hg[idx];
    if (b->count==0) {
        b->id=id; b->count=1; b->reward=4; return;
    }
    if (b->id==id) { b->count++; b->reward++; return; }
    if (b->reward>0) b->reward--;
    if (b->reward==0) {
        cms_add(b->id, b->count);
        cms_add(id, 1);
        b->id=id; b->count=1; b->reward=4;
        total_evictions++;
    }
}

/* ── exported API ───────────────────────────────────────── */
EXPORT void init_system(void) {
    memset(hg,  0, sizeof(hg));
    memset(cms, 0, sizeof(cms));
    total_evictions  = 0;
    active_guardians = 0;
}

EXPORT void free_system(void) {
    memset(hg,  0, sizeof(hg));
    memset(cms, 0, sizeof(cms));
    total_evictions  = 0;
    active_guardians = 0;
}

typedef struct {
    double   throughput;
    uint32_t active_guardians;
    uint32_t total_evictions;
} TelData;

EXPORT TelData process_chunk(uint32_t *ids, int n) {
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i=0;i<n;i++) insert_flow(ids[i]);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    double el = (t2.tv_sec-t1.tv_sec)+(t2.tv_nsec-t1.tv_nsec)/1e9;

    uint32_t ag=0;
    for (int i=0;i<HG_BUCKETS;i++) if(hg[i].count>0) ag++;
    active_guardians = ag;

    TelData td;
    td.throughput       = n/el;
    td.active_guardians = ag;
    td.total_evictions  = total_evictions;
    return td;
}

EXPORT uint32_t query_flow(uint32_t id) {
    uint32_t idx = hg_hash(id);
    if (hg[idx].id==id && hg[idx].count>0)
        return hg[idx].count;
    return cms_get(id);
}

EXPORT double get_mre(uint32_t *ids, uint32_t *true_counts, int n) {
    double err=0; int cnt=0;
    for (int i=0;i<n;i++) {
        if (true_counts[i]<5) continue;
        uint32_t est = query_flow(ids[i]);
        err += (double)abs((int)est-(int)true_counts[i])/true_counts[i];
        cnt++;
    }
    return cnt>0?err/cnt:0;
}