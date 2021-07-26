/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_STREAMMARKER_H
#define MINIFLAC_STREAMMARKER_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

typedef struct miniflac_streammarker_s miniflac_streammarker_t;
typedef enum MINIFLAC_STREAMMARKER_STATE MINIFLAC_STREAMMARKER_STATE;

enum MINIFLAC_STREAMMARKER_STATE {
    MINIFLAC_STREAMMARKER_F,
    MINIFLAC_STREAMMARKER_L,
    MINIFLAC_STREAMMARKER_A,
    MINIFLAC_STREAMMARKER_C,
};

struct miniflac_streammarker_s {
    MINIFLAC_STREAMMARKER_STATE state;
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_streammarker_init(miniflac_streammarker_t* streammarker);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streammarker_decode(miniflac_streammarker_t* streammarker, miniflac_bitreader_t* br);

#ifdef __cplusplus
}
#endif

#endif


