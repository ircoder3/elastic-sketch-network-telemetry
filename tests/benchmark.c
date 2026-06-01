#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../src/elastic_sketch.h"
#include "../src/count_min_sketch.h"
#include "../src/traffic_gen.h"

// Tests different sketch sizes and reports results
void run_benchmark(int heavy_buckets, int num_packets, double alpha) {
    TrafficData  *td = gen_zipfian_traffic(num_packets, 10000, alpha);
    ElasticSketch *es = es_create(heavy_buckets);

    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < td->count; i++)
        es_insert(es, td->packets[i]);
    clock_gettime(CLOCK_MONOTONIC, &t2);

    double elapsed = (t2.tv_sec  - t1.tv_sec)
                   + (t2.tv_nsec - t1.tv_nsec) / 1e9;

    printf("| %5d buckets | %8d pkts | alpha=%.1f | %.0f pkt/s |\n",
           heavy_buckets, num_packets, alpha, num_packets / elapsed);

    es_destroy(es);
    traffic_destroy(td);
}

int main() {
    printf("\n=== ELASTIC SKETCH BENCHMARK SUITE ===\n");
    printf("|   Config    |   Packets   |  Traffic  |  Throughput  |\n");
    printf("|-------------|-------------|-----------|---------------|\n");

    run_benchmark(512,  1000000, 1.0);
    run_benchmark(1024, 1000000, 1.5);
    run_benchmark(2048, 1000000, 1.5);
    run_benchmark(1024, 1000000, 2.0);

    printf("\nDone.\n");
    return 0;
}