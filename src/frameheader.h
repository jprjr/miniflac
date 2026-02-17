/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_FRAMEHEADER_H
#define MINIFLAC_FRAMEHEADER_H

#include <stdint.h>
#include <stddef.h>

#include "common.h"
#include "bitreader.h"

enum MINIFLAC_CHASSGN {
    MINIFLAC_CHASSGN_NONE,
    MINIFLAC_CHASSGN_LEFT_SIDE,
    MINIFLAC_CHASSGN_RIGHT_SIDE,
    MINIFLAC_CHASSGN_MID_SIDE,
};

enum MINIFLAC_FRAME_HEADER_STATE {
    MINIFLAC_FRAME_HEADER_SYNC,
    MINIFLAC_FRAME_HEADER_RESERVEBIT_1,
    MINIFLAC_FRAME_HEADER_BLOCKINGSTRATEGY,
    MINIFLAC_FRAME_HEADER_BLOCKSIZE,
    MINIFLAC_FRAME_HEADER_SAMPLERATE,
    MINIFLAC_FRAME_HEADER_CHANNELASSIGNMENT,
    MINIFLAC_FRAME_HEADER_SAMPLESIZE,
    MINIFLAC_FRAME_HEADER_RESERVEBIT_2,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_1,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_2,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_3,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_4,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_5,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_6,
    MINIFLAC_FRAME_HEADER_SAMPLENUMBER_7,
    MINIFLAC_FRAME_HEADER_BLOCKSIZE_MAYBE,
    MINIFLAC_FRAME_HEADER_SAMPLERATE_MAYBE,
    MINIFLAC_FRAME_HEADER_CRC8,
};

struct miniflac_frame_header_s {
    enum MINIFLAC_FRAME_HEADER_STATE state;
    uint8_t block_size_raw; /* block size value direct from header */
    uint8_t sample_rate_raw; /* sample rate value direct from header */
    uint8_t channel_assignment_raw; /* channel assignment value direct from header */
    uint8_t  blocking_strategy;
    uint16_t block_size; /* calculated/parsed block size */
    uint32_t sample_rate; /* calculated/parsed sample rate */
    enum MINIFLAC_CHASSGN channel_assignment;
    uint8_t  channels;
    uint8_t  bps;
    union {
        uint64_t sample_number;
        uint32_t frame_number;
    };
    uint8_t crc8;
    size_t size; /* size of the frame header, in bytes, only valid after sync */
};

typedef struct miniflac_frame_header_s miniflac_frame_header_t;
typedef enum MINIFLAC_CHASSGN MINIFLAC_CHASSGN;
typedef enum MINIFLAC_FRAME_HEADER_STATE MINIFLAC_FRAME_HEADER_STATE;

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_frame_header_init(miniflac_frame_header_t* frame_header);

MINIFLAC_PRIVATE
MINIFLAC_RESULT miniflac_frame_header_decode(miniflac_frame_header_t* frame_header, miniflac_bitreader_t* br);

#ifdef __cplusplus
}
#endif

#endif
