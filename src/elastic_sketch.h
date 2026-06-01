#ifndef ELASTIC_SKETCH_H
#define ELASTIC_SKETCH_H

#include "heavy_guardian.h"
#include "count_min_sketch.h"

typedef struct {
    HeavyGuardian  *heavy;
    CountMinSketch *light;
} ElasticSketch;

ElasticSketch* es_create(int heavy_buckets);
void     es_insert(ElasticSketch *es, uint32_t flow_id);
uint32_t es_query(ElasticSketch *es, uint32_t flow_id);
void     es_destroy(ElasticSketch *es);

#endif