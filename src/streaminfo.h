/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_STREAMINFO_H
#define MINIFLAC_STREAMINFO_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

/* public-facing streaminfo struct */
typedef struct miniflac_streaminfo_s miniflac_streaminfo_t;

typedef enum MINIFLAC_STREAMINFO_STATE MINIFLAC_STREAMINFO_STATE;

enum MINIFLAC_STREAMINFO_STATE {
    MINIFLAC_STREAMINFO_MINBLOCKSIZE,
    MINIFLAC_STREAMINFO_MAXBLOCKSIZE,
    MINIFLAC_STREAMINFO_MINFRAMESIZE,
    MINIFLAC_STREAMINFO_MAXFRAMESIZE,
    MINIFLAC_STREAMINFO_SAMPLERATE,
    MINIFLAC_STREAMINFO_CHANNELS,
    MINIFLAC_STREAMINFO_BPS,
    MINIFLAC_STREAMINFO_TOTALSAMPLES,
    MINIFLAC_STREAMINFO_MD5,
};

struct miniflac_streaminfo_s {
    MINIFLAC_STREAMINFO_STATE state;
    uint8_t                     pos;
    uint32_t            sample_rate;
    uint8_t                     bps;
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_streaminfo_init(miniflac_streaminfo_t* streaminfo);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_min_block_size(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint16_t* min_block_size);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_max_block_size(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint16_t* max_block_size);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_min_frame_size(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint32_t* min_frame_size);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_max_frame_size(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint32_t* max_frame_size);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_sample_rate(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint32_t* sample_rate);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_channels(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint8_t* channels);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_bps(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint8_t* bps);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_total_samples(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint64_t* total_samples);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_md5_length(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint32_t* md5_length);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_read_md5_data(miniflac_streaminfo_t* streaminfo, miniflac_bitreader_t* br, uint8_t* output, uint32_t length, uint32_t* outlen);

#ifdef __cplusplus
}
#endif

#endif

