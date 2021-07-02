/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_SUBFRAME_HEADER_H
#define MINIFLAC_SUBFRAME_HEADER_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

typedef struct miniflac_subframe_header_s miniflac_subframe_header;
typedef enum MINIFLAC_SUBFRAME_TYPE MINIFLAC_SUBFRAME_TYPE;
typedef enum MINIFLAC_SUBFRAME_HEADER_STATE MINIFLAC_SUBFRAME_HEADER_STATE;

enum MINIFLAC_SUBFRAME_TYPE {
    MINIFLAC_SUBFRAME_TYPE_UNKNOWN,
    MINIFLAC_SUBFRAME_TYPE_CONSTANT,
    MINIFLAC_SUBFRAME_TYPE_FIXED,
    MINIFLAC_SUBFRAME_TYPE_LPC,
    MINIFLAC_SUBFRAME_TYPE_VERBATIM,
};

enum MINIFLAC_SUBFRAME_HEADER_STATE {
    MINIFLAC_SUBFRAME_HEADER_RESERVEBIT1,
    MINIFLAC_SUBFRAME_HEADER_KIND,
    MINIFLAC_SUBFRAME_HEADER_WASTED_BITS,
    MINIFLAC_SUBFRAME_HEADER_UNARY,
};

struct miniflac_subframe_header_s {
    MINIFLAC_SUBFRAME_HEADER_STATE state;
    MINIFLAC_SUBFRAME_TYPE type;
    uint8_t order;
    uint8_t wasted_bits;
    uint8_t type_raw;
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_subframe_header_init(miniflac_subframe_header* subframeheader);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_subframe_header_decode(miniflac_subframe_header* subframeheader, miniflac_bitreader* br);

#ifdef __cplusplus
}
#endif

#endif

