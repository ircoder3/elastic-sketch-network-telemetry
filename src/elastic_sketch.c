#include "elastic_sketch.h"
#include <stdlib.h>

ElasticSketch* es_create(int heavy_buckets) {
    ElasticSketch *es = malloc(sizeof(ElasticSketch));
    es->heavy = hg_create(heavy_buckets);
    es->light = cms_create();
    return es;
}

void es_insert(ElasticSketch *es, uint32_t flow_id) {
    uint32_t evicted_id    = 0;
    uint32_t evicted_count = 0;

    int evicted = hg_insert(es->heavy, flow_id, &evicted_id, &evicted_count);

    if (evicted) {
        cms_update(es->light, evicted_id, evicted_count);
        cms_update(es->light, flow_id, 1);
    }
}

uint32_t es_query(ElasticSketch *es, uint32_t flow_id) {
    uint32_t heavy_count = hg_query(es->heavy, flow_id);
    uint32_t light_count = cms_query(es->light, flow_id);
    return (heavy_count > light_count) ? heavy_count : light_count;
}

void es_destroy(ElasticSketch *es) {
    hg_destroy(es->heavy);
    cms_destroy(es->light);
    free(es);
}