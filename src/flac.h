/* SPDX-License-Identifier: 0BSD */
#ifndef MINIFLAC_H
#define MINIFLAC_H

#include <stdint.h>
#include <stddef.h>

#include "common.h"
#include "bitreader.h"
#include "streammarker.h"
#include "metadata.h"
#include "frame.h"

typedef struct miniflac_s miniflac_t;
typedef enum MINIFLAC_STATE MINIFLAC_STATE;

enum MINIFLAC_STATE {
    MINIFLAC_UNKNOWN, /* will try to find a streammarker or frame sync code */
    MINIFLAC_STREAMMARKER, /* reading a stream marker */
    MINIFLAC_METADATA_OR_FRAME, /* will try to find a frame sync code or try to parse a metadata block */
    MINIFLAC_METADATA, /* currently reading a metadata block */
    MINIFLAC_FRAME,    /* currently reading an audio frame */
};

struct miniflac_s {
    MINIFLAC_STATE state;
    miniflac_bitreader br;
    miniflac_streammarker streammarker;
    miniflac_metadata metadata;
    miniflac_frame frame;
};


#ifdef __cplusplus
extern "C" {
#endif

/* returns the number of bytes needed for the miniflac struct (for malloc, etc) */
MINIFLAC_API
MINIFLAC_PURE
size_t
miniflac_size(void);

MINIFLAC_API
void
miniflac_init(miniflac_t* pFlac);

MINIFLAC_API
MINIFLAC_RESULT
miniflac_sync(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length);

MINIFLAC_API
MINIFLAC_RESULT
miniflac_decode(miniflac_t* pFlac, const uint8_t* data, uint32_t length, uint32_t* out_length, int32_t** samples);

#ifdef __cplusplus
}
#endif

#endif
