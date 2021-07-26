/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_SUBFRAME_FIXED_H
#define MINIFLAC_SUBFRAME_FIXED_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"
#include "residual.h"

typedef struct miniflac_subframe_fixed_s miniflac_subframe_fixed;
typedef enum MINIFLAC_SUBFRAME_FIXED_STATE MINIFLAC_SUBFRAME_FIXED_STATE;

enum MINIFLAC_SUBFRAME_FIXED_STATE {
    MINIFLAC_SUBFRAME_FIXED_DECODE,
};

struct miniflac_subframe_fixed_s {
    MINIFLAC_SUBFRAME_FIXED_STATE state;
    uint32_t pos;
    miniflac_residual residual;
};


#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_subframe_fixed_init(miniflac_subframe_fixed* c);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_fixed_decode(miniflac_subframe_fixed* c, miniflac_bitreader_t* br, int32_t* output, uint32_t block_size, uint8_t bps, uint8_t predictor_order);

#ifdef __cplusplus
}
#endif

#endif


