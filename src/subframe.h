/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_SUBFRAME_H
#define MINIFLAC_SUBFRAME_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"
#include "residual.h"
#include "subframeheader.h"
#include "subframe_constant.h"
#include "subframe_verbatim.h"
#include "subframe_fixed.h"
#include "subframe_lpc.h"

typedef struct miniflac_subframe_s miniflac_subframe_t;
typedef enum MINIFLAC_SUBFRAME_STATE MINIFLAC_SUBFRAME_STATE;

enum MINIFLAC_SUBFRAME_STATE {
    MINIFLAC_SUBFRAME_HEADER,
    MINIFLAC_SUBFRAME_CONSTANT,
    MINIFLAC_SUBFRAME_VERBATIM,
    MINIFLAC_SUBFRAME_FIXED,
    MINIFLAC_SUBFRAME_LPC,
};

struct miniflac_subframe_s {
    MINIFLAC_SUBFRAME_STATE state;
    uint8_t bps; /* effective bps for this subframe */
    miniflac_subframe_header_t header;
    miniflac_subframe_constant_t constant;
    miniflac_subframe_verbatim_t verbatim;
    miniflac_subframe_fixed_t fixed;
    miniflac_subframe_lpc_t lpc;
    miniflac_residual_t residual;
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_subframe_init(miniflac_subframe_t* subframe);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_decode(miniflac_subframe_t* subframe, miniflac_bitreader_t* br, int32_t* output, uint32_t block_size, uint8_t bps);


#ifdef __cplusplus
}
#endif

#endif
