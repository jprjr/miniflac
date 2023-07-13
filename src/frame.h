/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_FRAME_H
#define MINIFLAC_FRAME_H

#include <stdint.h>

#include "common.h"
#include "bitreader.h"

#include "streaminfo.h"
#include "frameheader.h"
#include "subframe.h"

enum MINIFLAC_FRAME_STATE {
    MINIFLAC_FRAME_HEADER,
    MINIFLAC_FRAME_SUBFRAME,
    MINIFLAC_FRAME_FOOTER,
};

/* represents an audio frame including parsed header */
struct miniflac_frame_s {
    enum MINIFLAC_FRAME_STATE state;
    uint8_t cur_subframe;
    uint16_t crc16;
    size_t size; /* size of the frame, in bytes, only valid after decode */
    struct miniflac_frame_header_s header;
    struct miniflac_subframe_s subframe;
};

typedef struct miniflac_frame_s miniflac_frame_t;
typedef enum MINIFLAC_FRAME_STATE MINIFLAC_FRAME_STATE;

#ifdef __cplusplus
extern "C" {
#endif

MINIFLAC_PRIVATE
void miniflac_frame_init(miniflac_frame_t* frame);

/* ensures we've just read the audio frame header and are ready to decode */
MINIFLAC_PRIVATE
MINIFLAC_RESULT miniflac_frame_sync(miniflac_frame_t* frame, miniflac_bitreader_t* br, miniflac_streaminfo_t* info);

MINIFLAC_PRIVATE
MINIFLAC_RESULT miniflac_frame_decode(miniflac_frame_t* frame, miniflac_bitreader_t* br, miniflac_streaminfo_t* info, int32_t** output);

#ifdef __cplusplus
}
#endif

#endif

