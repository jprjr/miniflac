/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_PADDING_H
#define MINIFLAC_PADDING_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

/* a padding block is supposed to be all zero bytes so
 * there's not a point in reading but - but who knows,
 * maybe somebody decides to do something weird with
 * padding blocks and shove data in there? */

typedef struct miniflac_padding_s miniflac_padding_t;

struct miniflac_padding_s {
    uint32_t len; /* length of data */
    uint32_t pos; /* current byte */
};

/* note: len is set outside of padding block functions */

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_padding_init(miniflac_padding_t* padding);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_padding_read_length(miniflac_padding_t* padding, miniflac_bitreader_t* br, uint32_t* length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_padding_read_data(miniflac_padding_t* padding, miniflac_bitreader_t* br, uint8_t* output, uint32_t length, uint32_t* outlen);

#ifdef __cplusplus
}
#endif

#endif
