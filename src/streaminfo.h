/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_STREAMINFO_H
#define MINIFLAC_STREAMINFO_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

/* public-facing streaminfo struct */
typedef struct miniflac_streaminfo_s miniflac_streaminfo_t;

typedef struct miniflac_streaminfo_private_s miniflac_streaminfo_private_t;
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
    MINIFLAC_STREAMINFO_MD5_1,
    MINIFLAC_STREAMINFO_MD5_2,
    MINIFLAC_STREAMINFO_MD5_3,
    MINIFLAC_STREAMINFO_MD5_4,
};

struct miniflac_streaminfo_s {
    uint16_t        min_block_size;
    uint16_t        max_block_size;
    uint32_t        min_frame_size;
    uint32_t        max_frame_size;
    uint32_t           sample_rate;
    uint8_t               channels;
    uint8_t                    bps;
    uint64_t         total_samples;
    uint8_t                md5[16];
};

struct miniflac_streaminfo_private_s {
    miniflac_streaminfo_t      info;
    MINIFLAC_STREAMINFO_STATE state;
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void
miniflac_streaminfo_init(miniflac_streaminfo_private_t* streaminfo);

MINIFLAC_PRIVATE
MINIFLAC_RESULT
miniflac_streaminfo_decode(miniflac_streaminfo_private_t* streaminfo, miniflac_bitreader* br, miniflac_streaminfo_t *out);

#ifdef __cplusplus
}
#endif

#endif

