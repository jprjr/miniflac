/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_SUBFRAME_CONSTANT_H
#define MINIFLAC_SUBFRAME_CONSTANT_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

typedef enum MINIFLAC_SUBFRAME_CONSTANT_STATE MINIFLAC_SUBFRAME_CONSTANT_STATE;
typedef struct miniflac_subframe_constant_s miniflac_subframe_constant_t;

enum MINIFLAC_SUBFRAME_CONSTANT_STATE {
    MINIFLAC_SUBFRAME_CONSTANT_DECODE,
};

struct miniflac_subframe_constant_s {
    MINIFLAC_SUBFRAME_CONSTANT_STATE state;
};


#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void miniflac_subframe_constant_init(miniflac_subframe_constant_t* c);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_constant_decode(miniflac_subframe_constant_t* c, miniflac_bitreader_t* br, int32_t* output, uint32_t block_size, uint8_t bps);

#ifdef __cplusplus
}
#endif

#endif
