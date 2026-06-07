// foundation.c
// MurmurHash3 + Zipfian traffic generator
#include "elastic_sketch.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// ── MurmurHash3 (32-bit) ─────────────────────────────────────────────────────
// Industry-standard non-cryptographic hash. Provides uniform distribution
// across the entire 32-bit output space. Time complexity: O(key_length).
// For our fixed 4-byte flow IDs this is always O(1).
uint32_t murmurhash3(const void *key, int len, uint32_t seed) {
    const uint32_t *data     = (const uint32_t *)key;
    const int       nblocks  = len / 4;
    uint32_t        h1       = seed;
    const uint32_t  c1       = 0xcc9e2d51;
    const uint32_t  c2       = 0x1b873593;

    // Body: process 4-byte blocks
    for (int i = 0; i < nblocks; i++) {
        uint32_t k1 = data[i];
        k1 *= c1;
        k1  = (k1 << 15) | (k1 >> 17);  // ROTL32(k1, 15)
        k1 *= c2;
        h1 ^= k1;
        h1  = (h1 << 13) | (h1 >> 19);  // ROTL32(h1, 13)
        h1  = h1 * 5 + 0xe6546b64;
    }

    // Tail: handle remaining bytes (< 4)
    const uint8_t *tail = (const uint8_t *)(data + nblocks);
    uint32_t k1 = 0;
    switch (len & 3) {
        case 3: k1 ^= (uint32_t)tail[2] << 16; /* fallthrough */
        case 2: k1 ^= (uint32_t)tail[1] <<  8; /* fallthrough */
        case 1: k1 ^= (uint32_t)tail[0];
                k1 *= c1;
                k1  = (k1 << 15) | (k1 >> 17);
                k1 *= c2;
                h1 ^= k1;
                break;
        default: break;
    }

    // Finalization mix – force all bits to avalanche
    h1 ^= (uint32_t)len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    return h1;
}

// ── Zipfian Distribution ─────────────────────────────────────────────────────
// Models real-world internet traffic: a tiny fraction of flows (heavy hitters)
// carry the vast majority of packets. Zipf(alpha=1.2) is standard for ISP data.
//
// We split into zipf_init() + zipf_sample() to avoid the fragile static-variable
// pattern. Call zipf_init() once, then call zipf_sample() as many times as needed.

static double *zipf_cdf  = NULL;   // Pre-computed CDF table
static int     zipf_n    = 0;      // Stored range for this session

// Build the CDF for Zipf(alpha, n). Returns 0 on success, -1 on alloc failure.
int zipf_init(double alpha, int n) {
    free(zipf_cdf);
    zipf_cdf = (double *)malloc((n + 1) * sizeof(double));
    if (!zipf_cdf) { fprintf(stderr, "zipf_init: malloc failed\n"); return -1; }

    // Normalisation constant: sum of 1/i^alpha for i=1..n
    double norm = 0.0;
    for (int i = 1; i <= n; i++) norm += 1.0 / pow((double)i, alpha);

    // Build cumulative distribution
    double running = 0.0;
    for (int i = 1; i <= n; i++) {
        running        += (1.0 / pow((double)i, alpha)) / norm;
        zipf_cdf[i]     = running;
    }
    zipf_cdf[0] = 0.0;
    zipf_n      = n;
    return 0;
}

// Binary-search the CDF for a single Zipf sample in O(log n).
int zipf_sample(void) {
    double z  = (double)rand() / ((double)RAND_MAX + 1.0);
    int    lo = 1, hi = zipf_n;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (zipf_cdf[mid] < z) lo = mid + 1;
        else                   hi = mid;
    }
    return lo;
}