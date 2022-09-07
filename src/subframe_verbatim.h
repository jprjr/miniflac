/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_SUBFRAME_VERBATIM_H
#define MINIFLAC_SUBFRAME_VERBATIM_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

enum MINIFLAC_SUBFRAME_VERBATIM_STATE {
    MINIFLAC_SUBFRAME_VERBATIM_DECODE,
};

struct miniflac_subframe_verbatim_s {
    enum MINIFLAC_SUBFRAME_VERBATIM_STATE state;
    uint32_t pos;
};

typedef struct miniflac_subframe_verbatim_s miniflac_subframe_verbatim_t;
typedef enum MINIFLAC_SUBFRAME_VERBATIM_STATE MINIFLAC_SUBFRAME_VERBATIM_STATE;

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_subframe_verbatim_init(miniflac_subframe_verbatim_t* c);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_verbatim_decode(miniflac_subframe_verbatim_t* c, miniflac_bitreader_t* br, int32_t* output, uint32_t block_size, uint8_t bps);

#ifdef __cplusplus
}
#endif

#endif

