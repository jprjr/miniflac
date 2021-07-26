/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_RESIDUAL_H
#define MINIFLAC_RESIDUAL_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

typedef struct miniflac_residual_s miniflac_residual;
typedef enum MINIFLAC_RESIDUAL_STATE MINIFLAC_RESIDUAL_STATE;

enum MINIFLAC_RESIDUAL_STATE {
    MINIFLAC_RESIDUAL_CODING_METHOD,
    MINIFLAC_RESIDUAL_PARTITION_ORDER,
    MINIFLAC_RESIDUAL_RICE_PARAMETER,
    MINIFLAC_RESIDUAL_RICE_SIZE, /* used when rice_parameter is an escape code */
    MINIFLAC_RESIDUAL_RICE_VALUE, /* used when rice_parameter is an escape code */
    MINIFLAC_RESIDUAL_MSB, /* used when reading MSB bits */
    MINIFLAC_RESIDUAL_LSB, /* used when reading MSB bits */
};

struct miniflac_residual_s {
    MINIFLAC_RESIDUAL_STATE state;
    uint8_t coding_method;
    uint8_t partition_order;
    uint8_t rice_parameter;
    uint8_t rice_size;
    uint32_t msb; /* unsure what the max for this is */
    uint8_t rice_parameter_size; /* 4 or 5 based on coding method */
    int32_t value; /* current residual value */

    uint32_t partition; /* current partition */
    uint32_t partition_total; /* total partitions */

    uint32_t residual; /* current residual within partition */
    uint32_t residual_total; /* total residuals in partition */
};


#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_residual_init(miniflac_residual* residual);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_residual_decode(miniflac_residual* residual, miniflac_bitreader_t* br, uint32_t* pos, uint32_t block_size, uint8_t predictor_order, int32_t *out);

#ifdef __cplusplus
}
#endif

#endif
