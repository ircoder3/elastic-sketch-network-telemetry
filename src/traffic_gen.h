#ifndef TRAFFIC_GEN_H
#define TRAFFIC_GEN_H

#include <stdint.h>

typedef struct {
    uint32_t *packets;   // Array of flow IDs
    int       count;     // Total packet count
    int       num_flows; // Unique flows
} TrafficData;

// alpha: skewness (1.5 = realistic internet traffic)
TrafficData* gen_zipfian_traffic(int num_packets, int num_flows, double alpha);
void traffic_destroy(TrafficData *td);

#endif