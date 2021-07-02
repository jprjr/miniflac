/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_FRAME_H
#define MINIFLAC_FRAME_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

#include "streaminfo.h"
#include "frameheader.h"
#include "subframe.h"

typedef struct miniflac_frame_s miniflac_frame;
typedef enum MINIFLAC_FRAME_STATE MINIFLAC_FRAME_STATE;

enum MINIFLAC_FRAME_STATE {
    MINIFLAC_FRAME_HEADER,
    MINIFLAC_FRAME_SUBFRAME,
    MINIFLAC_FRAME_FOOTER,
};

/* represents an audio frame including parsed header */
struct miniflac_frame_s {
    MINIFLAC_FRAME_STATE state;
    uint8_t cur_subframe;
    uint16_t crc16;
    miniflac_frame_header header;
    miniflac_subframe subframe;
};

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void miniflac_frame_init(miniflac_frame* frame);

/* ensures we've just read the audio frame header and are ready to decode */
MINIFLAC_PRIVATE
MINIFLAC_RESULT miniflac_frame_sync(miniflac_frame* frame, miniflac_bitreader* br, miniflac_streaminfo* info);

MINIFLAC_PRIVATE
MINIFLAC_RESULT miniflac_frame_decode(miniflac_frame* frame, miniflac_bitreader* br, miniflac_streaminfo* info, int32_t** output);

#ifdef __cplusplus
}
#endif

#endif

