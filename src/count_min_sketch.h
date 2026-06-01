#ifndef COUNT_MIN_SKETCH_H
#define COUNT_MIN_SKETCH_H

#include <stdint.h>

#define CMS_ROWS 4
#define CMS_COLS 8192

typedef struct {
    uint32_t table[CMS_ROWS][CMS_COLS];
} CountMinSketch;

CountMinSketch* cms_create();
void     cms_update(CountMinSketch *cms, uint32_t flow_id, uint32_t count);
uint32_t cms_query(CountMinSketch *cms, uint32_t flow_id);
void     cms_destroy(CountMinSketch *cms);

#endif