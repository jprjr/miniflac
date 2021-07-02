/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_SUBFRAME_LPC_H
#define MINIFLAC_SUBFRAME_LPC_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"
#include "residual.h"

typedef struct miniflac_subframe_lpc_s miniflac_subframe_lpc;
typedef enum MINIFLAC_SUBFRAME_LPC_STATE MINIFLAC_SUBFRAME_LPC_STATE;

enum MINIFLAC_SUBFRAME_LPC_STATE {
    MINIFLAC_SUBFRAME_LPC_PRECISION,
    MINIFLAC_SUBFRAME_LPC_SHIFT,
    MINIFLAC_SUBFRAME_LPC_COEFF,
};

struct miniflac_subframe_lpc_s {
    MINIFLAC_SUBFRAME_LPC_STATE state;
    uint32_t pos;
    uint8_t precision;
    uint8_t shift;
    uint8_t coeff;
    int32_t coefficients[32];
    miniflac_residual residual;
};


#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_subframe_lpc_init(miniflac_subframe_lpc* l);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_lpc_decode(miniflac_subframe_lpc* l, miniflac_bitreader* br, int32_t* output, uint32_t block_size, uint8_t bps, uint8_t predictor_order);

#ifdef __cplusplus
}
#endif

#endif



