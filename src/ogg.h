/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_OGG_H
#define MINIFLAC_OGG_H

#include <stdint.h>
#include "bitreader.h"
#include "common.h"

typedef struct miniflac_ogg_s miniflac_ogg_t;
typedef enum MINIFLAC_OGG_STATE MINIFLAC_OGG_STATE;

enum MINIFLAC_OGG_STATE {
    MINIFLAC_OGG_CAPTUREPATTERN_O,
    MINIFLAC_OGG_CAPTUREPATTERN_G1,
    MINIFLAC_OGG_CAPTUREPATTERN_G2,
    MINIFLAC_OGG_CAPTUREPATTERN_S,
    MINIFLAC_OGG_VERSION,
    MINIFLAC_OGG_HEADERTYPE,
    MINIFLAC_OGG_GRANULEPOS,
    MINIFLAC_OGG_SERIALNO,
    MINIFLAC_OGG_PAGENO,
    MINIFLAC_OGG_CHECKSUM,
    MINIFLAC_OGG_PAGESEGMENTS,
    MINIFLAC_OGG_SEGMENTTABLE,
    MINIFLAC_OGG_DATA,
    MINIFLAC_OGG_SKIP,
};

struct miniflac_ogg_s {
    MINIFLAC_OGG_STATE state;
    miniflac_bitreader_t br; /* maintain our own bitreader */
    uint8_t version;
    uint8_t headertype;
    int64_t granulepos;
    int32_t serialno;
    uint32_t pageno;
    uint8_t segments;
    uint8_t curseg; /* current position within the segment table */
    uint16_t length; /* length of data within page */
    uint16_t pos; /* where we are within page */
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_ogg_init(miniflac_ogg_t* ogg);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_ogg_sync(miniflac_ogg_t* ogg, miniflac_bitreader_t* br);

#ifdef __cplusplus
}
#endif

#endif
