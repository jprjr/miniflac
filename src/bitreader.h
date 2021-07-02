/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_BITREADER_H
#define MINIFLAC_BITREADER_H

#include <stdint.h>
#include "common.h"

typedef struct miniflac_bitreader_s miniflac_bitreader;

struct miniflac_bitreader_s {
    uint64_t val;
    uint8_t  bits;
    uint8_t  crc8;
    uint16_t crc16;
    uint32_t pos;
    uint32_t len;
    const uint8_t* buffer;
};


#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_bitreader_init(miniflac_bitreader* br);

MINIFLAC_PRIVATE
int
miniflac_bitreader_fill(miniflac_bitreader* br, uint8_t bits);

MINIFLAC_PRIVATE
uint64_t
miniflac_bitreader_read(miniflac_bitreader* br, uint8_t bits);

MINIFLAC_PRIVATE
int64_t
miniflac_bitreader_read_signed(miniflac_bitreader* br, uint8_t bits);

MINIFLAC_PRIVATE
uint64_t
miniflac_bitreader_peek(miniflac_bitreader* br, uint8_t bits);

MINIFLAC_PRIVATE
void
miniflac_bitreader_discard(miniflac_bitreader* br, uint8_t bits);

MINIFLAC_PRIVATE
void
miniflac_bitreader_align(miniflac_bitreader* br);

MINIFLAC_PRIVATE
void
miniflac_bitreader_reset_crc(miniflac_bitreader* br);

#ifdef __cplusplus
}
#endif

#endif





