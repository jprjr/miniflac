/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_SUBFRAME_VERBATIM_H
#define MINIFLAC_SUBFRAME_VERBATIM_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

typedef struct miniflac_subframe_verbatim_s miniflac_subframe_verbatim;
typedef enum MINIFLAC_SUBFRAME_VERBATIM_STATE MINIFLAC_SUBFRAME_VERBATIM_STATE;

enum MINIFLAC_SUBFRAME_VERBATIM_STATE {
    MINIFLAC_SUBFRAME_VERBATIM_DECODE,
};

struct miniflac_subframe_verbatim_s {
    MINIFLAC_SUBFRAME_VERBATIM_STATE state;
    uint32_t pos;
};


#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_subframe_verbatim_init(miniflac_subframe_verbatim* c);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_verbatim_decode(miniflac_subframe_verbatim* c, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps);

#ifdef __cplusplus
}
#endif

#endif

