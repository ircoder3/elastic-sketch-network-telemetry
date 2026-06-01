#include "traffic_gen.h"
#include <stdlib.h>
#include <math.h>

TrafficData* gen_zipfian_traffic(int num_packets, int num_flows, double alpha) {
    // Step 1: Calculate Zipfian probabilities
    // P(rank k) = (1/k^alpha) / sum(1/i^alpha for i=1..N)
    double *probs = (double*)malloc(num_flows * sizeof(double));
double  norm  = 0.0;
if (!probs) return NULL;
probs[0]      = 0.0;   // initialize first element — silences the warning

    for (int i = 1; i <= num_flows; i++) {
        probs[i-1] = 1.0 / pow((double)i, alpha);
        norm += probs[i-1];
    }
    for (int i = 0; i < num_flows; i++)
        probs[i] /= norm;

    // Step 2: Build cumulative distribution (CDF)
    double *cdf = malloc(num_flows * sizeof(double));
    cdf[0] = probs[0];
    for (int i = 1; i < num_flows; i++)
        cdf[i] = cdf[i-1] + probs[i];

    // Step 3: Sample packets using inverse CDF
    uint32_t *packets = malloc(num_packets * sizeof(uint32_t));
    srand(42); // Fixed seed for reproducibility

    for (int p = 0; p < num_packets; p++) {
        double r = (double)rand() / RAND_MAX;
        // Binary search on CDF
        int lo = 0, hi = num_flows - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (cdf[mid] < r) lo = mid + 1;
            else              hi = mid;
        }
        packets[p] = (uint32_t)(lo + 1); // flow IDs start at 1
    }

    free(probs);
    free(cdf);

    TrafficData *td = malloc(sizeof(TrafficData));
    td->packets   = packets;
    td->count     = num_packets;
    td->num_flows = num_flows;
    return td;
}

void traffic_destroy(TrafficData *td) {
    free(td->packets);
    free(td);
}