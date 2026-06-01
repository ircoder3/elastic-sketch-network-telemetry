#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include "elastic_sketch.h"
#include "traffic_gen.h"

#define NUM_PACKETS      1000000
#define NUM_FLOWS        10000
#define ZIPF_ALPHA       1.5
#define HEAVY_BUCKETS    2048
#define HEAVY_THRESHOLD  500
#define MRE_MIN_COUNT    10

int main() {
    printf("====================================\n");
    printf(" ELASTIC SKETCH - NETWORK TELEMETRY\n");
    printf("====================================\n\n");

    // --- STEP 1: Generate Traffic ---
    printf("[*] Generating %d packets (Zipfian, alpha=%.1f)...\n",
           NUM_PACKETS, ZIPF_ALPHA);
    TrafficData *traffic = gen_zipfian_traffic(NUM_PACKETS, NUM_FLOWS, ZIPF_ALPHA);

    // --- STEP 2: Ground Truth ---
    uint32_t *ground_truth = calloc(NUM_FLOWS + 1, sizeof(uint32_t));
    for (int i = 0; i < traffic->count; i++)
        ground_truth[traffic->packets[i]]++;

    // --- STEP 3: Process through Elastic Sketch ---
    printf("[*] Processing packets through Elastic Sketch...\n\n");
    ElasticSketch *es = es_create(HEAVY_BUCKETS);

    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    for (int i = 0; i < traffic->count; i++)
        es_insert(es, traffic->packets[i]);

    clock_gettime(CLOCK_MONOTONIC, &t_end);

    // --- STEP 4: Throughput ---
    double elapsed    = (t_end.tv_sec  - t_start.tv_sec)
                      + (t_end.tv_nsec - t_start.tv_nsec) / 1e9;
    double throughput = NUM_PACKETS / elapsed;

    // --- STEP 5: Accuracy Metrics ---
    int    true_positives  = 0;
    int    false_positives = 0;
    int    actual_heavy    = 0;
    double total_mre       = 0.0;
    int    mre_count       = 0;

    // Track per-tier MRE
    double mre_heavy = 0.0; int mre_heavy_count = 0;
    double mre_mid   = 0.0; int mre_mid_count   = 0;
    double mre_light = 0.0; int mre_light_count = 0;

    for (int f = 1; f <= NUM_FLOWS; f++) {
        uint32_t estimated = es_query(es, (uint32_t)f);
        uint32_t actual    = ground_truth[f];

        if (actual > HEAVY_THRESHOLD) actual_heavy++;

        // Precision / Recall
        if (estimated > HEAVY_THRESHOLD) {
            if (actual > HEAVY_THRESHOLD) true_positives++;
            else                          false_positives++;
        }

        // MRE — only for flows seen enough times to be meaningful
        if (actual >= MRE_MIN_COUNT) {
            double rel_err = (double)abs((int)estimated - (int)actual)
                           / (double)actual;
            total_mre += rel_err;
            mre_count++;

            // Track by tier
            if (actual > HEAVY_THRESHOLD) {
                mre_heavy += rel_err; mre_heavy_count++;
            } else if (actual > 50) {
                mre_mid   += rel_err; mre_mid_count++;
            } else {
                mre_light += rel_err; mre_light_count++;
            }
        }
    }

    double precision = (true_positives + false_positives > 0)
                     ? (double)true_positives / (true_positives + false_positives)
                     : 0.0;
    double recall    = (actual_heavy > 0)
                     ? (double)true_positives / actual_heavy
                     : 0.0;
    double mre       = (mre_count > 0) ? total_mre / mre_count : 0.0;

    // --- STEP 6: Print Results ---
    printf("--- BENCHMARK RESULTS ---\n");
    printf("Total Packets Processed : %d\n",      NUM_PACKETS);
    printf("Time Elapsed            : %.3f sec\n", elapsed);
    printf("Throughput              : %.0f pkt/s\n\n", throughput);

    printf("--- ACCURACY RESULTS ---\n");
    printf("Heavy Hitters Found     : %d (actual: %d)\n",
           true_positives + false_positives, actual_heavy);
    printf("Precision               : %.1f%%\n", precision * 100.0);
    printf("Recall                  : %.1f%%\n", recall    * 100.0);
    printf("\n");
    printf("Mean Relative Error     : %.4f (%.2f%%)  [flows >= %d packets]\n",
           mre, mre * 100.0, MRE_MIN_COUNT);
    printf("  Heavy flows  (>%4d)  : %.2f%%  (%d flows)\n",
           HEAVY_THRESHOLD,
           mre_heavy_count ? (mre_heavy/mre_heavy_count)*100 : 0,
           mre_heavy_count);
    printf("  Mid flows    (>  50)  : %.2f%%  (%d flows)\n",
           mre_mid_count   ? (mre_mid  /mre_mid_count  )*100 : 0,
           mre_mid_count);
    printf("  Light flows  (<= 50)  : %.2f%%  (%d flows)\n",
           mre_light_count ? (mre_light/mre_light_count)*100 : 0,
           mre_light_count);
    printf("\n");

    printf("--- MEMORY USAGE ---\n");
    printf("Heavy Guardian          : %d buckets  (~%llu KB)\n",
           HEAVY_BUCKETS,
           (unsigned long long)(HEAVY_BUCKETS * sizeof(HeavyBucket)) / 1024);
    printf("Count-Min Sketch        : %d x %d     (~%llu KB)\n",
           CMS_ROWS, CMS_COLS,
           (unsigned long long)(CMS_ROWS * CMS_COLS * sizeof(uint32_t)) / 1024);
    printf("====================================\n");

    // Cleanup
    es_destroy(es);
    traffic_destroy(traffic);
    free(ground_truth);
    return 0;
}